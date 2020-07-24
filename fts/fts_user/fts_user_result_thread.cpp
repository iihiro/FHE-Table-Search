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

#include <memory>
#include <fstream>
#include <vector>
#include <cstring>
#include <stdsc/stdsc_client.hpp>
#include <stdsc/stdsc_buffer.hpp>
#include <stdsc/stdsc_packet.hpp>
#include <stdsc/stdsc_log.hpp>
#include <stdsc/stdsc_exception.hpp>
#include <fts_share/fts_packet.hpp>
#include <fts_user/fts_user_cs_client.hpp>
#include <fts_user/fts_user_result_thread.hpp>

namespace fts_user
{
    
template <class T>
struct ResultThread<T>::Impl
{
    std::shared_ptr<stdsc::ThreadException> te_;

    Impl(CSClient& client, cbfunc_t cbfunc, void* cbargs)
        : client_(client),
          cbfunc_(cbfunc),
          cbargs_(cbargs)
    {
        te_ = stdsc::ThreadException::create();
    }

    void exec(T& args, std::shared_ptr<stdsc::ThreadException> te)
    {
        try
        {
            STDSC_LOG_INFO("launched result thread for query#%d", args.query_id);

            fts_share::EncData enc_result(params);
            client_.recv_results(args.query_id, enc_result);
            STDSC_LOG_TRACE("get result of query#%d", args.query_id);
            cbfunc_(enc_result, cbargs_);
        }
        catch (const stdsc::AbstractException& e)
        {
            STDSC_LOG_TRACE("Failed to client process (%s)", e.what());
            te->set_current_exception();
        }
    }

private:
    CSClient& client_;
    cbfunc_t cbfunc_;
    void* cbargs_;
};

template <class T>
ResultThread<T>::ResultThread(CSClient& cs_client, cbfunc_t cbfunc, void* cbargs)
    : pimpl_(new Impl(cs_client, cbfunc, cbargs))
{
}

template <class T>
ResultThread<T>::~ResultThread(void)
{
    super::join();
}

template <class T>
void ResultThread<T>::start(T& param)
{
    super::start(param, pimpl_->te_);
}

template <class T>
void ResultThread<T>::wait(void)
{
    super::join();
    pimpl_->te_->rethrow_if_has_exception();
}

template <class T>
void ResultThread<T>::exec(T& args,
                        std::shared_ptr<stdsc::ThreadException> te) const
{
    try
    {
        pimpl_->exec(args, te);
    }
    catch (...)
    {
        te->set_current_exception();
    }
}

template class ResultThread<ResultThreadParam>;

} /* namespace fts_user */
