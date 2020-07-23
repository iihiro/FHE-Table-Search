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

#ifndef FTS_CLIENT_CS_CLIENT_HPP
#define FTS_CLIENT_CS_CLIENT_HPP

#include <memory>
#include <fts_share/fts_define.hpp>

namespace fts_share
{
    class EncData;
}

namespace fts_user
{

/**
 * @brief Provides client for Computation Server.
 */
class CSClient
{
public:
    /**
     * constructor
     * @param[in] host hostname
     * @param[in] port port number
     */
    CSClient(const char* host, const char* port);
    virtual ~CSClient(void) = default;

    /**
     * connect
     * @param[in] retry_interval_usec retry interval (usec)
     * @param[in] timeout_sec timeout (sec)
     */
    void connect(const uint32_t retry_interval_usec = FTS_RETRY_INTERVAL_USEC,
                 const uint32_t timeout_sec = FTS_TIMEOUT_SEC);
    /**
     * disconnect
     */
    void disconnect();
    
    /**
     * send query
     * @param[in] key_id keyID
     * @param[in] func_no function number
     * @param[in] params parameters for seal
     * @param[in] enc_input encrypted input values (1 or 2)
     * @return queryID
     */
    int32_t send_query(const int32_t key_id, const int32_t func_no,
                       const seal::EncryptionParameters& params,
                       const fts_share::EncData& enc_inputs) const;
    //int32_t send_query(const int32_t key_id, const int32_t func_no, const std::vector<fts_share::EncData>& enc_input) const;
    
    /**
     * receive results
     * @param[in] query_id queryID
     * @param[out] enc_result encrypted result
     * @return success or falied
     */
    bool recv_result(const int32_t query_id, fts_share::EncData& enc_result) const;

private:
    struct Impl;
    std::shared_ptr<Impl> pimpl_;
};

} /* namespace fts_user */

#endif /* FTS_USER_CS_CLIENT_HPP */
