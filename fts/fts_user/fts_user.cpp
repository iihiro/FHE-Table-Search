/*
 * Copyright 2018 Yamana Laboratory, Waseda University
 * Supported by JST CREST Grant Number JPMJCR1503, Japan.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE‐2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <memory>
#include <fstream>
#include <vector>
#include <cstring>
#include <cmath>
#include <stdsc/stdsc_client.hpp>
#include <stdsc/stdsc_buffer.hpp>
#include <stdsc/stdsc_packet.hpp>
#include <stdsc/stdsc_log.hpp>
#include <stdsc/stdsc_exception.hpp>
#include <fts_share/fts_encdata.hpp>
#include <fts_share/fts_seal_utility.hpp>
#include <fts_user/fts_user_dec_client.hpp>
#include <fts_user/fts_user_cs_client.hpp>
#include <fts_user/fts_user.hpp>

#include <seal/seal.h>

namespace fts_user
{

struct User::Impl
{
    Impl(const std::string& dec_host, const std::string& dec_port,
         const std::string& cs_host, const std::string& cs_port,
         const uint32_t retry_interval_usec,
         const uint32_t timeout_sec)
        : dec_host_(dec_host),
          dec_port_(dec_port),
          cs_host_(cs_host),
          cs_port_(cs_port),
          retry_interval_usec_(retry_interval_usec),
          timeout_sec_(timeout_sec)
    {}

    ~Impl(void)
    {}

    int32_t new_keys()
    {
        DecClient dec_client(dec_host_.c_str(), dec_port_.c_str());
        dec_client.connect(retry_interval_usec_, timeout_sec_);
        auto key_id = dec_client.new_keys(seckey_);
        STDSC_LOG_INFO("generate new keys. (key_id:%d)", key_id);
        fts_share::seal_utility::write_to_file("seckey.txt", seckey_);
        return key_id;
    }
    
    void compute(int32_t key_id, const int64_t val)
    {
        DecClient dec_client(dec_host_.c_str(), dec_port_.c_str());
        dec_client.connect(retry_interval_usec_, timeout_sec_);
        
        seal::PublicKey pubkey;
        dec_client.get_pubkey(key_id, pubkey);
        fts_share::seal_utility::write_to_file("pubkey.txt", pubkey);

        seal::GaloisKeys galoiskey;
        dec_client.get_galoiskey(key_id, galoiskey);
        fts_share::seal_utility::write_to_file("galoiskey.txt", galoiskey);

        seal::RelinKeys relinkey;
        dec_client.get_relinkey(key_id, relinkey);
        fts_share::seal_utility::write_to_file("relinkey.txt", relinkey);
        
        seal::EncryptionParameters params(seal::scheme_type::BFV);
        dec_client.get_param(key_id, params);
        fts_share::seal_utility::write_to_file("param.txt", params);
        //dbg_dumpparam_to_file("param.txt", params);

        CSClient cs_client(cs_host_.c_str(), cs_port_.c_str());
        cs_client.connect(retry_interval_usec_, timeout_sec_);

        //std::vector<fts_share::EncData> enc_inputs;
        fts_share::EncData enc_inputs(params);
        enc_inputs.encrypt(val, pubkey, galoiskey);
        //enc_inputs.push_back(enc_data);

        // experiment
        // この実験ができたので、次回はCSへクエリを送って、それを受け取るCallbackを各ところから
        {
            // save to file
            fts_share::seal_utility::write_to_file("enc_input.txt", enc_inputs.data());
            
            // save to stream
            auto sz = enc_inputs.stream_size();
            stdsc::BufferStream buffstream(sz);
            std::iostream stream(&buffstream);
            enc_inputs.save_to_stream(stream);

            // load from stream
            fts_share::EncData enc_inputs2(params);
            enc_inputs2.load_from_stream(stream);

            // decrypt
            int64_t output_value;
            enc_inputs2.decrypt(seckey_, output_value);
            printf("  -- %ld\n", output_value);
        }
        
        const int32_t func_no = 111;
        //auto query_id = cs_client.send_query(key_id, func_no, enc_inputs);
        auto query_id = cs_client.send_query(key_id, func_no, params, enc_inputs);
        printf("query_id: %d\n", query_id);
    }

//private:
//    template <class T>
//    void fts_share::seal_utility::write_to_file(const std::string& filepath, const T& data)
//    {
//        std::ofstream ofs(filepath, std::ios::binary);
//        data.save(ofs);
//        ofs.close();
//    }
//    
//    void dbg_dumpparam_to_file(const std::string& filepath,
//                               const seal::EncryptionParameters& param)
//    {
//        std::ofstream ofs(filepath, std::ios::binary);
//        seal::EncryptionParameters::Save(param, ofs);
//        ofs.close();
//    }

//    void encrypt(const int64_t val,
//                 const seal::EncryptionParameters& params,
//                 const seal::PublicKey& public_key,
//                 const seal::GaloisKeys& galoiskey,
//                 std::vector<fts_share::EncData> enc_input)
//    {
//        //resetting FHE
//        //cout << "Setting FHE..." << flush;
//        //ifstream parmsFile("Params");
//        //EncryptionParameters parms(scheme_type::BFV);
//        //parms = EncryptionParameters::Load(parmsFile);
//        auto context = seal::SEALContext::Create(params);
//        //parmsFile.close();
//        //
//        //ifstream pkFile("PublicKey");
//        //PublicKey public_key;
//        //public_key.unsafe_load(pkFile);
//        //pkFile.close();
//        //
//        //ifstream galFile("GaloisKey");
//        //GaloisKeys gal_keys;
//        //gal_keys.unsafe_load(galFile);
//        //galFile.close();
//
//        seal::Encryptor encryptor(context, public_key);
//        seal::Evaluator evaluator(context);
//        seal::BatchEncoder batch_encoder(context);
//
//        size_t slot_count = batch_encoder.slot_count();
//        size_t row_size = slot_count / 2;
//        std::cout << "Plaintext matrix row size: " << row_size << std::endl;
//        std::cout << "Slot nums = " << slot_count << std::endl;
//
//        //int64_t l = row_size;
//        //int64_t k = std::ceil(FTS_COMMONPARAM_TABLE_SIZE_N / row_size);
//
//        unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
//        std::mt19937 generator(seed);
//
//        //string s1(argv[1]);
//        //make query
//        std::cout<<"Get query!";
//        // string LUT_query_temp;
//        // cin >> LUT_query_temp;
//        // int64_t LUT_query=std::atoi(LUT_query_temp);
//        int64_t LUT_query = val;
//
//        //encrypt the LUT query
//        std::cout << "Encrypt and save your query..." << std::flush;
//        std::vector<int64_t> query;
//        for(size_t i=0 ; i<row_size ; i++){
//            query.push_back(LUT_query);
//        }
//        query.resize(slot_count);
//
//        /*
//          Printing the matrix is a bit of a pain.
//        */
//        auto print_matrix = [row_size](auto &matrix) {
//            std::cout << std::endl;
//            size_t print_size = 5;
//                
//            std::cout << "    [";
//            for (size_t i = 0; i < print_size; i++) {
//                std::cout << std::setw(3) << matrix[i] << ",";
//            }
//            std::cout << std::setw(3) << " ...,";
//            for (size_t i = row_size - print_size; i < row_size; i++) {
//                std::cout << std::setw(3) << matrix[i] << ((i != row_size - 1) ? "," : " ]\n");
//            }
//            std::cout << "    [";
//            for (size_t i = row_size; i < row_size + print_size; i++) {
//                //std::cout<<i<<std::endl<<row_size<<std::endl<<print_size<<std::endl;
//                std::cout << std::setw(3) << matrix[i] << ",";
//            }
//            std::cout << std::setw(3) << " ...,";
//            for (size_t i = 2 * row_size - print_size; i < 2 * row_size; i++) {
//                std::cout << std::setw(3) << matrix[i] << ((i != 2 * row_size - 1) ? "," : " ]\n");
//            }
//            std::cout << std::endl;
//        };
//        seal::Plaintext plaintext_query;
//        batch_encoder.encode(query, plaintext_query);
//        //encoder the vector to a coefficient
//        //std::cout << "Plaintext polynomial: " << plaintext_query.to_string() << std::endl;
//        print_matrix(query);
//
//        seal::Ciphertext ciphertext_query;
//        std::cout << "Encrypting: ";
//        encryptor.encrypt(plaintext_query, ciphertext_query);
//        std::cout << "Done" << std::endl;
//
//        //save in a file
//        std::cout << "Save in a file." << std::endl;
//        std::ofstream queryFile;
//        queryFile.open("query", std::ios::binary);
//        ciphertext_query.save(queryFile);
//        queryFile.close();
//        std::cout << "End" << std::endl;
//    }
    
private:
    const std::string dec_host_;
    const std::string dec_port_;
    const std::string cs_host_;
    const std::string cs_port_;
    const uint32_t retry_interval_usec_;
    const uint32_t timeout_sec_;
    seal::SecretKey seckey_;
};

User::User(const std::string& dec_host, const std::string& dec_port,
           const std::string& cs_host, const std::string& cs_port,
           const uint32_t retry_interval_usec,
           const uint32_t timeout_sec)
    : pimpl_(new Impl(dec_host, dec_port, cs_host, cs_port,
                      retry_interval_usec, timeout_sec))
{
}

int32_t User::new_keys()
{
    return pimpl_->new_keys();
}

void User::compute(const int32_t key_id, const int64_t val)
{
    pimpl_->compute(key_id, val);
}

} /* namespace fts_user */
