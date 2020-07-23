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

#ifndef FTS_CS_RESULT_HPP
#define FTS_CS_RESULT_HPP

#include <cstdint>
#include <cstdbool>
//#include <fts_share/fts_concurrent_queue.hpp>
#include <fts_share/fts_concurrent_mapqueue.hpp>
#include <seal/seal.h>

namespace fts_cs
{

struct Result
{
    Result(const int32_t query_id, const seal::Ciphertext& ctxt);
    virtual ~Result() = default;

    int32_t query_id_;
    seal::Ciphertext ctxt_;
};

#if 1
struct ResultQueue : public fts_share::ConcurrentMapQueue<int32_t, Result>
{
    using super = fts_share::ConcurrentMapQueue<int32_t, Result>;
    
    ResultQueue() = default;
    virtual ~ResultQueue() = default;
};
#else
struct ResultQueue : public fts_share::ConcurrentQueue<Result>
{
    using super = fts_share::ConcurrentQueue<Result>;
    
    ResultQueue() = default;
    virtual ~ResultQueue() = default;

    bool is_exist(const int32_t query_id) const;
};
#endif

} /* namespace fts_cs */

#endif /* FTS_CS_RESULT_HPP */
