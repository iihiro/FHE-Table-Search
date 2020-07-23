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

#ifndef FTS_CS_QUERY_HPP
#define FTS_CS_QUERY_HPP

#include <cstdint>
#include <vector>
#include <fts_share/fts_concurrent_queue.hpp>
#include <seal/seal.h>

namespace fts_cs
{

struct Query
{
    Query(const int32_t key_id, const int32_t func_no,
          const std::vector<seal::Ciphertext>& ctxts);
    virtual ~Query() = default;

    const int32_t key_id_;
    const int32_t func_no_;
    std::vector<seal::Ciphertext> ctxts_;
};

struct QueryQeueu : public fts_share::ConcurrentQueue<Query>
{
    using super = fts_share::ConcurrentQueue<Query>;
    
    QueryQeueu() = default;
    virtual ~QueryQeueu() = default;

    virtual int32_t regist(const Query& data)
    {
        super::push(data);
        return 12333;
    }
};


} /* namespace fts_cs */

#endif /* FTS_CS_QUERY_HPP */
