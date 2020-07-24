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

#ifndef FTS_CONCURRENT_QUEUE_HPP
#define FTS_CONCURRENT_QUEUE_HPP

#include <vector>
#include <mutex>
#include <cstdbool>

namespace fts_share
{

template <class T>
class ConcurrentQueue
{
public:
    ConcurrentQueue() = default;
    virtual ~ConcurrentQueue() = default;

    virtual void push(const T& data)
    {
        std::lock_guard<std::mutex> lock(mtx_);
        vec_.push_back(data);
    }
    
    virtual bool pop(T& data)
    {
        if (0 == vec_.size()) {
            return false;
        }
        
        std::lock_guard<std::mutex> lock(mtx_);
        data = vec_[0];
        vec_.erase(vec_.begin());
        return true;
    }

    virtual size_t size()
    {
        std::lock_guard<std::mutex> lock(mtx_);
        return vec_.size();
    }

private:
    std::vector<T> vec_;
    std::mutex mtx_;
};

} /* namespace fts_share */

#endif /* FTS_CONCURRENT_QUEUE_HPP */