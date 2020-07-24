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

#ifndef FTS_USER_RESULT_THREAD_HPP
#define FTS_USER_RESULT_THREAD_HPP

#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <stdsc/stdsc_thread.hpp>
#include <fts_user/fts_user_result_cbfunc.hpp>

namespace fts_user
{
    
class ResultThreadParam;
class CSClient;

/**
 * @brief The thread to receive result from CS.
 */
template <class T = ResultThreadParam>
class ResultThread : public stdsc::Thread<T>
{
    using super = stdsc::Thread<T>;

public:    
    ResultThread(CSClient& cs_client, cbfunc_t cbfunc, void* cbargs);
    virtual ~ResultThread(void);

    void start(T& param);
    void wait(void);

private:
    virtual void exec(T& args,
                      std::shared_ptr<stdsc::ThreadException> te) const override;

    struct Impl;
    std::shared_ptr<Impl> pimpl_;
};

struct ResultThreadParam
{
    int32_t query_id;
};

} /* namespace fts_user */

#endif /* FTS_USER_RESULT_THREAD_HPP */
