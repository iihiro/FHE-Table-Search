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
#include <fts_share/fts_csparam.hpp>
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

        CSClient cs_client(cs_host_.c_str(), cs_port_.c_str());
        cs_client.connect(retry_interval_usec_, timeout_sec_);

        fts_share::EncData enc_inputs(params);
        enc_inputs.encrypt(val, pubkey, galoiskey);

        // experiment
        // この実験ができたので、次回はCSへクエリを送って、それを受け取るCallbackを各ところから
        {
            // save to file
            fts_share::seal_utility::write_to_file("query.txt", enc_inputs.data());
            
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
        
        auto query_id = cs_client.send_query(key_id, fts_share::kFuncOne, params, enc_inputs);
        printf("query_id: %d\n", query_id);
    }

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
