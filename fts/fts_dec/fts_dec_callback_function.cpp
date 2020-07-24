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

#include <iostream>
#include <cstring>
#include <fstream>
#include <stdsc/stdsc_buffer.hpp>
#include <stdsc/stdsc_state.hpp>
#include <stdsc/stdsc_socket.hpp>
#include <stdsc/stdsc_packet.hpp>
#include <stdsc/stdsc_exception.hpp>
#include <fts_share/fts_packet.hpp>
#include <fts_share/fts_plaindata.hpp>
#include <fts_share/fts_commonparam.hpp>
#include <fts_share/fts_cs2decparam.hpp>
#include <fts_share/fts_encdata.hpp>
#include <fts_share/fts_seal_utility.hpp>
#include <fts_dec/fts_dec_callback_function.hpp>
#include <fts_dec/fts_dec_callback_param.hpp>
#include <fts_dec/fts_dec_keycontainer.hpp>
#include <fts_dec/fts_dec_state.hpp>

namespace fts_dec
{

// CallbackFunction for new key Request
DEFUN_DOWNLOAD(CallbackFunctionNewKeyRequest)
{
    STDSC_LOG_INFO("Received new key request. (current state : %s)",
                   state.current_state_str().c_str());

    DEF_CDATA_ON_EACH(fts_dec::CallbackParam);
    DEF_CDATA_ON_ALL(fts_dec::CommonCallbackParam);
    auto& param   = cdata_e->param;
    auto& keycont = cdata_a->keycont;

    auto key_id = keycont.new_keys(param);

    fts_share::PlainData<int32_t> plaindata;
    plaindata.push(key_id);

    auto key_id_sz  = plaindata.stream_size();
    auto seckey_sz = keycont.size(key_id, KeyKind_t::kKindSecKey);
    auto total_sz  = seckey_sz + key_id_sz;
    
    stdsc::BufferStream buffstream(total_sz);
    std::iostream stream(&buffstream);

    plaindata.save_to_stream(stream);

    seal::SecretKey seckey;
    keycont.get(key_id, KeyKind_t::kKindSecKey, seckey);
    seckey.save(stream);
    
    STDSC_LOG_INFO("Sending key_id and secret key.");
    stdsc::Buffer* bsbuff = &buffstream;
    sock.send_packet(stdsc::make_data_packet(fts_share::kControlCodeDataNewKeys, total_sz));
    sock.send_buffer(*bsbuff);
    state.set(kEventNewKeysRequest);
}

// CallbackFunction for Pubkey Request
DEFUN_UPDOWNLOAD(CallbackFunctionPubKeyRequest)
{
    STDSC_LOG_INFO("Received public key request. (current state : %s)",
                   state.current_state_str().c_str());

    DEF_CDATA_ON_ALL(fts_dec::CommonCallbackParam);
    auto& keycont = cdata_a->keycont;

    auto key_id = *static_cast<const int32_t*>(buffer.data());
    STDSC_LOG_INFO("public key request with key_id: %d", key_id);

    auto sz = keycont.size(key_id, KeyKind_t::kKindPubKey);
    stdsc::BufferStream buffstream(sz);
    std::iostream stream(&buffstream);

    seal::PublicKey pubkey;
    keycont.get(key_id, KeyKind_t::kKindPubKey, pubkey);
    pubkey.save(stream);

    STDSC_LOG_INFO("Sending publick key.");
    stdsc::Buffer* bsbuff = &buffstream;
    sock.send_packet(stdsc::make_data_packet(fts_share::kControlCodeDataPubKey, sz));
    sock.send_buffer(*bsbuff);
    state.set(kEventPubKeyRequest);
}

// CallbackFunction for Galoiskey Request
DEFUN_UPDOWNLOAD(CallbackFunctionGaloisKeyRequest)
{
    STDSC_LOG_INFO("Received galois key request. (current state : %s)",
                   state.current_state_str().c_str());

    DEF_CDATA_ON_ALL(fts_dec::CommonCallbackParam);
    auto& keycont = cdata_a->keycont;

    auto key_id = *static_cast<const int32_t*>(buffer.data());
    STDSC_LOG_INFO("galois key request with key_id: %d", key_id);

    auto sz = keycont.size(key_id, KeyKind_t::kKindGaloisKey);
    stdsc::BufferStream buffstream(sz);
    std::iostream stream(&buffstream);

    seal::GaloisKeys galoiskey;
    keycont.get(key_id, KeyKind_t::kKindGaloisKey, galoiskey);
    galoiskey.save(stream);

    STDSC_LOG_INFO("Sending galois key.");
    stdsc::Buffer* bsbuff = &buffstream;
    sock.send_packet(stdsc::make_data_packet(fts_share::kControlCodeDataGaloisKey, sz));
    sock.send_buffer(*bsbuff);
    state.set(kEventGaloisKeyRequest);
}

// CallbackFunction for Relinkey Request
DEFUN_UPDOWNLOAD(CallbackFunctionRelinKeyRequest)
{
    STDSC_LOG_INFO("Received relin key request. (current state : %s)",
                   state.current_state_str().c_str());

    DEF_CDATA_ON_ALL(fts_dec::CommonCallbackParam);
    auto& keycont = cdata_a->keycont;

    auto key_id = *static_cast<const int32_t*>(buffer.data());
    STDSC_LOG_INFO("relin key request with key_id: %d", key_id);

    auto sz = keycont.size(key_id, KeyKind_t::kKindRelinKey);
    stdsc::BufferStream buffstream(sz);
    std::iostream stream(&buffstream);

    seal::RelinKeys relinkey;
    keycont.get(key_id, KeyKind_t::kKindRelinKey, relinkey);
    relinkey.save(stream);

    STDSC_LOG_INFO("Sending relin key.");
    stdsc::Buffer* bsbuff = &buffstream;
    sock.send_packet(stdsc::make_data_packet(fts_share::kControlCodeDataRelinKey, sz));
    sock.send_buffer(*bsbuff);
    state.set(kEventRelinKeyRequest);
}

// CallbackFunction for Param Request
DEFUN_UPDOWNLOAD(CallbackFunctionParamRequest)
{
    STDSC_LOG_INFO("Received param request. (current state : %s)",
                   state.current_state_str().c_str());

    DEF_CDATA_ON_ALL(fts_dec::CommonCallbackParam);
    auto& keycont = cdata_a->keycont;

    auto key_id = *static_cast<const int32_t*>(buffer.data());
    STDSC_LOG_INFO("param request with key_id: %d", key_id);

    auto sz = keycont.size(key_id, KeyKind_t::kKindParam);
    printf("hogefuga: %ld\n", sz);
    stdsc::BufferStream buffstream(sz);
    std::iostream stream(&buffstream);

    seal::EncryptionParameters params(seal::scheme_type::BFV);
    keycont.get_param(key_id, params);
    seal::EncryptionParameters::Save(params, stream);

    STDSC_LOG_INFO("Sending param.");
    stdsc::Buffer* bsbuff = &buffstream;
    sock.send_packet(stdsc::make_data_packet(fts_share::kControlCodeDataParam, sz));
    sock.send_buffer(*bsbuff);
    state.set(kEventParamRequest);
}

// CallbackFunction for Delete key Request
DEFUN_DATA(CallbackFunctionDeleteKeyRequest)
{
    STDSC_LOG_INFO("Received delete key request. (current state : %s)",
                   state.current_state_str().c_str());

    DEF_CDATA_ON_ALL(fts_dec::CommonCallbackParam);
    auto& keycont = cdata_a->keycont;

    auto key_id = *static_cast<const int32_t*>(buffer.data());
    STDSC_LOG_INFO("delete key request with key_id: %d", key_id);

    keycont.delete_keys(key_id);
    state.set(kEventDeleteKeysRequest);
}

static std::vector<int64_t> shift_work(const std::vector<int64_t>& query,
                                       const int64_t index,
                                       const int64_t num_slots)
{
    std::vector<int64_t> new_index;
    int64_t size = query.size();
    int64_t temp;

    for (int64_t i=0; i<num_slots; ++i) {
        if ((i+index) >= size) {
            temp = query[i + index - size];
        } else {
            temp = query[i + index];
        }
        new_index.push_back(temp);
    }
    return new_index;
}
    
static void compute(const std::vector<seal::Ciphertext>& midresults,
                    const seal::SecretKey& seckey,
                    const seal::PublicKey& pubkey,
                    const seal::EncryptionParameters& params,
                    seal::Ciphertext& new_PIR_query,
                    seal::Ciphertext& new_PIR_index)
{
    auto context = seal::SEALContext::Create(params);

    seal::Encryptor encryptor(context, pubkey);
    seal::Evaluator evaluator(context);
    seal::Decryptor decryptor(context, seckey);

    seal::BatchEncoder batch_encoder(context);
    size_t slot_count = batch_encoder.slot_count();
    size_t row_size = slot_count / 2;
    std::cout << "Plaintext matrix row size: " << row_size << std::endl;
    std::cout << "Slot nums = " << slot_count << std::endl;

    int64_t l = row_size;
    int64_t k = ceil(FTS_COMMONPARAM_TABLE_SIZE_N / row_size);

    const auto& ct_result = midresults;
    std::vector<std::vector<int64_t>> dec_result(k);
    std::vector<seal::Plaintext> poly_dec_result;
    for (int s=0; s<k; ++s) {
        seal::Plaintext ex;
        poly_dec_result.push_back(ex);
    }
    std::cout << "Reading intermediate result..." << std::flush;
    //auto start2=chrono::high_resolution_clock::now();

  //string s1(argv[1]);
  //ifstream mid_result(s1+'c', ios::binary);
  //Ciphertext temp;
  //for(int w = 0; w < k ; w++) {
  //  temp.load(context, mid_result);
  //  ct_result.push_back(temp);
  //}
  //mid_result.close();
  //cout << "OK" << endl;

  // auto end2=chrono::high_resolution_clock::now();
  // chrono::duration<double> diff2 = end2 - start2;
  // cout << "Reading time is: " << diff2.count() << "s" << endl;

    std::cout << "Decrypting..."<< std::flush;

  //auto start1=chrono::high_resolution_clock::now();

    //#pragma omp parallel for num_threads(NF)
    //omp_set_num_threads(NF);
    //#pragma omp parallel for
    for (int z=0; z<k; ++z) {
        //cout<<"i: "<<z<<endl;
        decryptor.decrypt(ct_result[z], poly_dec_result[z]);
        batch_encoder.decode(poly_dec_result[z], dec_result[z]);
    }

    //NTL_EXEC_RANGE_END

    std::cout << "OK" << std::endl;

    // auto end1=chrono::high_resolution_clock::now();
    // chrono::duration<double> diff = end1-start1;
    // cout << "Decryption time is: " << diff.count() << "s" << endl;

    //output_plaintext(dec_result);
///////////////////////////////////////////////////////////////////
    //find the position of 0.
    std::cout << "Making PIR-query..." << std::flush;
    //auto start3=chrono::high_resolution_clock::now();

    int64_t index;
    std::vector<int64_t> new_query;
    int64_t flag=0;

    for (int i=0; i<k; ++i) {
        for (size_t j=0; j<row_size; ++j) {
            if (dec_result[i][j] == 0 && flag == 0) {
                index = i;
                flag = 1;
                std::cout << "i: " << i << " j: " << j<< std::endl;
                for (size_t kk=0; kk<static_cast<size_t>(l); ++kk) {
                    if (kk==j) new_query.push_back(1);
                    else new_query.push_back(0);
                }
            }
        }
    }
    if(flag == 0) {
        std::cout << "ERROR: NO FIND INPUT NUMBER!" << std::endl;
    }
    std::cout << "OK" << std::endl;

    //new_index is new_query left_shift the value of index
    std::vector<int64_t> new_index;
    //new_query.resize(slot_count);
    new_index = shift_work(new_query, index, row_size);
    std::cout << "index is " << index << std::endl;
    // out(new_index);
    // out(new_query);
    new_query.resize(slot_count);
    new_index.resize(slot_count);

    //encrypt new query
    std::cout << "Encrypting..." << std::flush;
    //seal::Ciphertext new_PIR_query;
    seal::Plaintext plaintext_new_PIR_query;
    batch_encoder.encode(new_query, plaintext_new_PIR_query);

    //seal::Ciphertext new_PIR_index;
    seal::Plaintext plaintext_new_PIR_index;
    batch_encoder.encode(new_index, plaintext_new_PIR_index);

    encryptor.encrypt(plaintext_new_PIR_query, new_PIR_query);
    encryptor.encrypt(plaintext_new_PIR_index, new_PIR_index);

    std::cout << "OK" << std::endl;
    // auto end3=chrono::high_resolution_clock::now();
    // chrono::duration<double> diff3 = end3-start3;
    // cout << "Make PIR query time is: " << diff3.count() << "s" << endl;

    //write in a file
    {
        std::cout << "Saving query..." << std::flush;
        std::ofstream PIRqueryFile;
        PIRqueryFile.open("queryd", std::ios::binary);
        new_PIR_query.save(PIRqueryFile);
        PIRqueryFile.close();

        std::ofstream PIRindexFile;
        PIRindexFile.open("queryi", std::ios::binary);
        new_PIR_index.save(PIRindexFile);
        PIRindexFile.close();
        
        std::cout << "OK" << std::endl;
    }

    //auto endWhole=chrono::high_resolution_clock::now();
    //chrono::duration<double> diffWhole = endWhole-startWhole;
    //cout << "Whole runtime is: " << diffWhole.count() << "s" << endl;
    
}
    
// CallbackFunction for Query
DEFUN_UPDOWNLOAD(CallbackFunctionCsMidResult)
{
    STDSC_LOG_INFO("Received Cs mid-result. (current state : %s)",
                   state.current_state_str().c_str());

    DEF_CDATA_ON_ALL(fts_dec::CommonCallbackParam);
    auto& keycont = cdata_a->keycont;

    stdsc::BufferStream rbuffstream(buffer);
    std::iostream rstream(&rbuffstream);

    // load plaindata (param)
    fts_share::PlainData<fts_share::Cs2DecParam> rplaindata;
    rplaindata.load_from_stream(rstream);
    const auto param = rplaindata.data();
    STDSC_LOG_INFO("query with key_id: %d, query_id: %d", param.key_id, param.query_id);

    // load encryption parameters
    //seal::EncryptionParameters params(seal::scheme_type::BFV);
    //params = seal::EncryptionParameters::Load(rstream);
    seal::SecretKey seckey;
    seal::PublicKey pubkey;
    seal::EncryptionParameters params(seal::scheme_type::BFV);
    keycont.get(param.key_id, KeyKind_t::kKindSecKey, seckey);
    keycont.get(param.key_id, KeyKind_t::kKindPubKey, pubkey);
    keycont.get_param(param.key_id, params);

    // load encryption mid-result
    fts_share::EncData enc_midresult(params);
    enc_midresult.load_from_stream(rstream);
    fts_share::seal_utility::write_to_file("queryc.txt", enc_midresult.data());

    seal::Ciphertext new_PIR_query;
    seal::Ciphertext new_PIR_index;
    compute(enc_midresult.vdata(), seckey, pubkey, params,
            new_PIR_query, new_PIR_index);
    
    //Query query(param.key_id, param.func_no, enc_inputs.vdata());
    //int32_t query_id = calc_manager.put(query);
    //
    //fts_share::PlainData<int32_t> splaindata;
    //splaindata.push(query_id);
    //
    fts_share::EncData enc_PIRquery(params, new_PIR_query);
    fts_share::EncData enc_PIRindex(params, new_PIR_index);
    
    auto sz = enc_PIRquery.stream_size() + enc_PIRindex.stream_size();
    stdsc::BufferStream sbuffstream(sz);
    std::iostream sstream(&sbuffstream);
    
    enc_PIRquery.save_to_stream(sstream);
    enc_PIRindex.save_to_stream(sstream);
    
    STDSC_LOG_INFO("Sending PIR queries.");
    stdsc::Buffer* bsbuff = &sbuffstream;
    sock.send_packet(stdsc::make_data_packet(fts_share::kControlCodeDataCsMidResult, sz));
    sock.send_buffer(*bsbuff);
    state.set(kEventCsMidResult);
}

} /* namespace fts_dec */
