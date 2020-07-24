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

#ifndef FTS_CONCURRENT_MAPQUEUE_HPP
#define FTS_CONCURRENT_MAPQUEUE_HPP

#include <map>
#include <mutex>
#include <cstdbool>
#include <stdsc/stdsc_exception.hpp>

namespace fts_share
{

template <class Tk, class Tv>
class ConcurrentMapQueue
{
public:
    ConcurrentMapQueue() = default;
    virtual ~ConcurrentMapQueue() = default;

    virtual void push(const Tk& key, const Tv& val)
    {
        STDSC_THROW_INVPARAM_IF_CHECK(!map_.count(key), "key has already exist.");
        std::lock_guard<std::mutex> lock(mtx_);
        map_.emplace(key, val);
    }
    
    virtual bool pop(Tk& key, Tv& val)
    {
        std::lock_guard<std::mutex> lock(mtx_);

        if (0 == map_.size()) {
            return false;
        }

        const auto front = map_.begin();
        key = front->first;
        val = front->second;
        map_.erase(front);
        return true;
    }

    virtual bool get(const Tk& key, Tv& val)
    {
        if (0 == map_.size() || 0 == map_.count(key)) {
            return false;
        }
        
        std::lock_guard<std::mutex> lock(mtx_);
        val = map_.at(key);
        return true;        
    }

private:
    std::map<Tk, Tv> map_;
    std::mutex mtx_;
};

} /* namespace fts_share */

#endif /* FTS_CONCURRENT_MAPQUEUE_HPP */
