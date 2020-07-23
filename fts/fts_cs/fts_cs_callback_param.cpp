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

//#include <fts_cs/fts_cs_query.hpp>
//#include <fts_cs/fts_cs_result.hpp>
#include <fts_cs/fts_cs_calcmanager.hpp>
#include <fts_cs/fts_cs_callback_param.hpp>

namespace fts_cs
{

// CallbackParam
CallbackParam::CallbackParam(void)
{
}

// CommonCallbackparam
CommonCallbackParam::CommonCallbackParam()
    : calc_manager(new CalcManager())
//    : query_queue(new QueryQueue()),
//      result_queue(new ResultQueue())
{}

} /* namespace fts_cs */
