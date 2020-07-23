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

#ifndef FTS_CS_CALCTHREAD_HPP
#define FTS_CS_CALCTHREAD_HPP

#include <memory>
#include <stdsc/stdsc_thread.hpp>

namespace fts_cs
{
    
class CalcThreadParam;
class QueryQueue;
class ResultQueue;

/**
 * @brief Calculation thread
 */
class CalcThread : public stdsc::Thread<CalcThreadParam>
{
    using super = Thread<CalcThreadParam>;
public:
    CalcThread(const QueryQueue& in_queue, ResultQueue& out_queue);
    virtual ~CalcThread(void) = default;

    void start();
    void stop();
    
private:
    virtual void exec(CalcThreadParam& args,
                      std::shared_ptr<stdsc::ThreadException> te) const override;

    struct Impl;
    std::shared_ptr<Impl> pimpl_;
};

/**
 * @brief This class is used to hold the calcration parameters for CalcThread.
 */
struct CalcThreadParam
{
    int32_t dummy;
};

} /* namespace fts_cs */

#endif /* FTS_CS_CALCTHREAD_HPP */
