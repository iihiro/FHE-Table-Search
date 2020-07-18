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
#include <fts_user/fts_user_dec_client.hpp>
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
    {
        //dec_client_->connect(retry_interval_usec_, timeout_sec_);
        //eval_client_->connect(retry_interval_usec_, timeout_sec_);
        //
        //context_ = std::make_shared<fts_share::Context>();
        //dec_client_->get_context(*context_);
        //
        //pubkey_ = std::make_shared<fts_share::PubKey>(context_->get());
        //dec_client_->get_pubkey(*pubkey_);
        //
        //dec_client_->get_param(dec_param_);
        //
        //STDSC_LOG_INFO("param: logN:%lu, bit_len:%lu",
        //               dec_param_.logN, dec_param_.bit_len);
    }

    ~Impl(void)
    {
        //dec_client_->disconnect();
    }

    int32_t new_keys()
    {
        DecClient dec_client(dec_host_.c_str(), dec_port_.c_str());
        dec_client.connect(retry_interval_usec_, timeout_sec_);
        return dec_client.new_keys(seckey_);
    }
    
    void compute(int32_t keyID, const int64_t val)
    {
        DecClient dec_client(dec_host_.c_str(), dec_port_.c_str());
        dec_client.connect(retry_interval_usec_, timeout_sec_);
        
        seal::PublicKey pubkey;
        dec_client.get_pubkey(keyID, pubkey);
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

void User::compute(const int32_t keyID, const int64_t val)
{
    pimpl_->compute(keyID, val);
}

} /* namespace fts_user */
