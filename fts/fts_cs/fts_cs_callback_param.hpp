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

#ifndef FTS_CS_CALLBACK_PARAM_HPP
#define FTS_CS_CALLBACK_PARAM_HPP

//#include <fts_share/fts_concurrent_queue.hpp>
#include <fts_cs/fts_cs_query.hpp>

namespace fts_cs
{

/**
 * @brief This class is used to hold the callback parameters for Decryptor.
 */
struct CallbackParam
{
    CallbackParam(void);
    ~CallbackParam(void) = default;
};

/**
 * @brief This class is used to hold the callback parameters for Decryptor
 * This parameter to shared on all connections.
 */
struct CommonCallbackParam
{
    //fts_share::ConcurrentQueue<Query> query_queue;
    QueryQeueu query_queue;
};

} /* namespace fts_cs */

#endif /* FTS_CS_CALLBACK_PARAM_HPP */
