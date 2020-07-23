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

#ifndef FTS_CS_CALCMANAGER_HPP
#define FTS_CS_CALCMANAGER_HPP

#include <memory>
#include <cstdbool>

namespace fts_cs
{

class Query;

class CalcManager
{
public:
    CalcManager();
    virtual ~CalcManager() = default;

    void start_threads(const uint32_t thread_num);
    void stop_threads();
    
    int32_t push_query(const Query& query);
    bool pop_result(const int32_t query_id, Result& result) const;

private:
    class Impl;
    std::shared_ptr<Impl> pimpl_;
};

} /* namespace fts_cs */

#endif /* FTS_CS_CALCMANAGER_HPP */
