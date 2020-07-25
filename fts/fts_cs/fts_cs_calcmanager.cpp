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
#include <fstream>
#include <stdsc/stdsc_log.hpp>
#include <stdsc/stdsc_exception.hpp>
#include <fts_share/fts_utility.hpp>
#include <fts_cs/fts_cs_query.hpp>
#include <fts_cs/fts_cs_result.hpp>
#include <fts_cs/fts_cs_calcthread.hpp>
#include <fts_cs/fts_cs_calcmanager.hpp>


namespace fts_cs
{
    void read_table(const std::string& filepath, std::vector<std::vector<int64_t>>& table)
    {
        int64_t temp;
        std::string lineStr;

        STDSC_LOG_INFO("Read LUT file. (filepath:%s)", filepath);

        if (!fts_share::utility::file_exist(filepath)) {
            std::ostringstream oss;
            oss << "File not found. (" << filepath << ")";
            STDSC_THROW_FILE(oss.str());
        }

        std::ifstream ifs(filepath, std::ios::in);
        while (getline(ifs, lineStr)){
            std::vector<int64_t> table_col;
            std::stringstream ss(lineStr);
            std::string str;
            while (getline(ss, str, ' ')){
                temp = std::stoi(str);
                table_col.push_back(temp);
            }
            table.push_back(table_col);
        }
    }
    
    struct CalcManager::Impl
    {
        Impl(const std::string& LUT_filepath,
             const uint32_t max_concurrent_queries,
             const uint32_t max_results,
             const uint32_t result_lifetime_sec)
            : max_concurrent_queries_(max_concurrent_queries),
              max_results_(max_results),
              result_lifetime_sec_(result_lifetime_sec)
        {
            read_table(LUT_filepath, oriLUT_);
        }

        const uint32_t max_concurrent_queries_;
        const uint32_t max_results_;
        const uint32_t result_lifetime_sec_;
        QueryQueue qque_;
        ResultQueue rque_;
        std::vector<std::vector<int64_t>> oriLUT_;
        std::vector<std::shared_ptr<CalcThread>> threads_;
    };

    CalcManager::CalcManager(const std::string& LUT_filepath,
                             const uint32_t max_concurrent_queries,
                             const uint32_t max_results,
                             const uint32_t result_lifetime_sec)
        :pimpl_(new Impl(LUT_filepath,
                         max_concurrent_queries,
                         max_results,
                         result_lifetime_sec))
    {}

    void CalcManager::start_threads(const uint32_t thread_num,
                                    const std::string& dec_host,
                                    const std::string& dec_port)
    {
        STDSC_LOG_INFO("Start calculation threads. (n:%d)", thread_num);
        pimpl_->threads_.clear();
        for (size_t i=0; i<thread_num; ++i) {
            pimpl_->threads_.emplace_back(
                std::make_shared<CalcThread>(pimpl_->qque_,
                                             pimpl_->rque_,
                                             pimpl_->oriLUT_,
                                             dec_host,
                                             dec_port));
        }

        for (const auto& thread : pimpl_->threads_) {
            thread->start();
        }
    }
    
    void CalcManager::stop_threads()
    {
        STDSC_LOG_INFO("Stop calculation threads.");
    }
    
    int32_t CalcManager::push_query(const Query& query)
    {
        STDSC_LOG_INFO("Set queries.");
        int32_t query_id = -1;
        try {
            query_id = pimpl_->qque_.push(query);
        } catch (stdsc::AbstractException& ex) {
            STDSC_LOG_WARN(ex.what());
        }
            
        return query_id;
    }

    void CalcManager::pop_result(const int32_t query_id, Result& result,
                                 const uint32_t retry_interval_msec) const
    {
        STDSC_LOG_INFO("Getting results of query. (retry_interval_msec: %u ms)", retry_interval_msec);
        while (!pimpl_->rque_.pop(query_id, result)) {
            usleep(retry_interval_msec * 1000);
        }
    }

} /* namespace fts_cs */
