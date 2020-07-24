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

#include <vector>
#include <iomanip> // for setw
#include <stdsc/stdsc_exception.hpp>
#include <stdsc/stdsc_log.hpp>
#include <fts_share/fts_utility.hpp>
#include <fts_share/fts_commonparam.hpp>
#include <fts_share/fts_encdata.hpp>

namespace fts_share
{

struct EncData::Impl
{
    explicit Impl(const seal::EncryptionParameters& params)
        : params_(params)
    {}

    const seal::EncryptionParameters& params_;
};

EncData::EncData(const seal::EncryptionParameters& params)
    : pimpl_(new Impl(params))
{}

EncData::EncData(const seal::EncryptionParameters& params, const seal::Ciphertext& ctxt)
    : pimpl_(new Impl(params))
{
    vec_.push_back(ctxt);
}

void EncData::encrypt(const int64_t input_value,
                      const seal::PublicKey& pubkey,
                      const seal::GaloisKeys& galoiskey)
{
    auto context = seal::SEALContext::Create(pimpl_->params_);

    seal::Encryptor encryptor(context, pubkey);
    seal::Evaluator evaluator(context);
    seal::BatchEncoder batch_encoder(context);

    size_t slot_count = batch_encoder.slot_count();
    size_t row_size = slot_count / 2;
    std::cout << "Plaintext matrix row size: " << row_size << std::endl;
    std::cout << "Slot nums = " << slot_count << std::endl;

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    std::mt19937 generator(seed);

    //encrypt the LUT query
    std::cout << "Encrypt and save your query..." << std::flush;
    std::vector<int64_t> query;
    for(size_t i=0 ; i<row_size ; i++){
        query.push_back(input_value);
    }
    query.resize(slot_count);

    /*
      Printing the matrix is a bit of a pain.
    */
    auto print_matrix = [row_size](auto &matrix) {
        std::cout << std::endl;
        size_t print_size = 5;
            
        std::cout << "    [";
        for (size_t i = 0; i < print_size; i++) {
            std::cout << std::setw(3) << matrix[i] << ",";
        }
        std::cout << std::setw(3) << " ...,";
        for (size_t i = row_size - print_size; i < row_size; i++) {
            std::cout << std::setw(3) << matrix[i] << ((i != row_size - 1) ? "," : " ]\n");
        }
        std::cout << "    [";
        for (size_t i = row_size; i < row_size + print_size; i++) {
            std::cout << std::setw(3) << matrix[i] << ",";
        }
        std::cout << std::setw(3) << " ...,";
        for (size_t i = 2 * row_size - print_size; i < 2 * row_size; i++) {
            std::cout << std::setw(3) << matrix[i] << ((i != 2 * row_size - 1) ? "," : " ]\n");
        }
        std::cout << std::endl;
    };
    seal::Plaintext plaintext_query;
    batch_encoder.encode(query, plaintext_query);
    print_matrix(query);

    seal::Ciphertext ciphertext_query;
    std::cout << "Encrypting: ";
    encryptor.encrypt(plaintext_query, ciphertext_query);
    std::cout << "Done" << std::endl;

    //save in a file
#if 1
    std::cout << "Save in a file." << std::endl;
    std::ofstream queryFile;
    queryFile.open("query", std::ios::binary);
    ciphertext_query.save(queryFile);
    queryFile.close();
#endif
    std::cout << "End" << std::endl;

    vec_.push_back(ciphertext_query);
}

void EncData::decrypt(const seal::SecretKey& secret_key,
                      int64_t& output_value) const
{
    auto context = seal::SEALContext::Create(pimpl_->params_);
    
    seal::Decryptor decryptor(context, secret_key);
    seal::BatchEncoder batch_encoder(context);

    std::vector<int64_t> query;
    seal::Plaintext plaintext_query;
    decryptor.decrypt(vec_[0], plaintext_query);

    std::vector<int64_t> outputs;
    batch_encoder.decode(plaintext_query, outputs);
    if (0 < outputs.size()) {
        output_value = outputs[0];
    }
}

void EncData::save_to_stream(std::ostream& os) const
{
    size_t sz = vec_.size();
    os.write(reinterpret_cast<char*>(&sz), sizeof(sz));

    for (const auto& v : vec_) {
        v.save(os);
    }
}
             
void EncData::load_from_stream(std::istream& is)
{
    size_t sz;
    is.read(reinterpret_cast<char*>(&sz), sizeof(sz));
    
    clear();

    auto context = seal::SEALContext::Create(pimpl_->params_);
    
    for (size_t i=0; i<sz; ++i) {
        seal::Ciphertext ciphertext_query;
        ciphertext_query.load(context, is);
        vec_.push_back(ciphertext_query);
    }
}

void EncData::save_to_file(const std::string& filepath) const
{
    std::ofstream ofs(filepath);
    save_to_stream(ofs);
    ofs.close();
}

void EncData::load_from_file(const std::string& filepath)
{
    if (!fts_share::utility::file_exist(filepath)) {
        std::ostringstream oss;
        oss << "File not found. (" << filepath << ")";
        STDSC_THROW_FILE(oss.str());
    }
    std::ifstream ifs(filepath, std::ios::binary);
    load_from_stream(ifs);
    ifs.close();
}

} /* namespace fts_share */
