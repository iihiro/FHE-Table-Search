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

#include <fts_cs/fts_cs_result.hpp>
#include <seal/seal.h>

namespace fts_cs
{

// Result
Result::Result(const int32_t query_id, const seal::Ciphertext& ctxt)
    : query_id_(query_id), ctxt_(ctxt)
{
}

// ResultQueue
bool ResultQueue::try_get(const int32_t query_id, Result& result)
{
    return super::get(query_id, result);
}

} /* namespace fts_cs */
