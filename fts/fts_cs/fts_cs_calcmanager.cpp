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

#include <vector>
#include <unistd.h>
#include <stdsc/stdsc_log.hpp>
#include <fts_cs/fts_cs_query.hpp>
#include <fts_cs/fts_cs_result.hpp>
#include <fts_cs/fts_cs_calcthread.hpp>
#include <fts_cs/fts_cs_calcmanager.hpp>


namespace fts_cs
{
    struct CalcManager::Impl
    {
        Impl(const uint32_t max_concurrent_queries,
             const uint32_t max_results,
             const uint32_t result_lifetime_sec)
            : max_concurrent_queries_(max_concurrent_queries),
              max_results_(max_results),
              result_lifetime_sec_(result_lifetime_sec)
        {}

        const uint32_t max_concurrent_queries_;
        const uint32_t max_results_;
        const uint32_t result_lifetime_sec_;
        QueryQueue qque_;
        ResultQueue rque_;
        std::vector<std::shared_ptr<CalcThread>> threads_;
    };

    CalcManager::CalcManager(const uint32_t max_concurrent_queries,
                             const uint32_t max_results,
                             const uint32_t result_lifetime_sec)
        :pimpl_(new Impl(max_concurrent_queries,
                         max_results,
                         result_lifetime_sec))
    {}

    void CalcManager::start_threads(const uint32_t thread_num,
                                    const std::string& dec_host,
                                    const std::string& dec_port)
    {
        pimpl_->threads_.clear();
        for (size_t i=0; i<thread_num; ++i) {
            pimpl_->threads_.emplace_back(std::make_shared<CalcThread>(pimpl_->qque_,
                                                                       pimpl_->rque_,
                                                                       dec_host,
                                                                       dec_port));
        }

        for (const auto& thread : pimpl_->threads_) {
            thread->start();
        }
    }
    
    void CalcManager::stop_threads()
    {
    }
    
    int32_t CalcManager::put(const Query& query)
    {
        int32_t query_id = -1;
        try {
            query_id = pimpl_->qque_.push(query);
        } catch (stdsc::AbstractException& ex) {
            printf(" 2a");
            STDSC_LOG_WARN(ex.what());
            printf(" 2b");
        }
            
        return query_id;
    }

    void CalcManager::get(const int32_t query_id, Result& result,
                          const uint32_t retry_interval_msec) const
    {
        while (!try_get(query_id, result)) {
            usleep(retry_interval_msec * 1000);
        }
    }
    
    bool CalcManager::try_get(const int32_t query_id, Result& result) const
    {
        return pimpl_->rque_.try_get(query_id, result);
    }
    
} /* namespace fts_cs */
