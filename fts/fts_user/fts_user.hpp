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

#ifndef FTS_USER_HPP
#define FTS_USER_HPP

#include <memory>
#include <vector>
#include <fts_share/fts_define.hpp>

namespace fts_user
{
    
/**
 * @brief Provides encryptor.
 */
class User
{
public:
    
    User(const char* dec_host, const char* dec_port,
         const char* cs_host, const char* cs_port,
         const uint32_t retry_interval_usec = FTS_RETRY_INTERVAL_USEC,
         const uint32_t timeout_sec = FTS_TIMEOUT_SEC);
    virtual ~User(void) = default;

    int32_t new_keys();
    void compute(const int32_t keyID, const int64_t val);
    
private:
    struct Impl;
    std::shared_ptr<Impl> pimpl_;
};

} /* namespace fts_user */

#endif /* FTS_USER_HPP */
