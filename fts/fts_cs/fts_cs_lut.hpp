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

#ifndef FTS_CS_LUT_HPP
#define FTS_CS_LUT_HPP

#include <string>
#include <unordered_map>
#include <sstream>

namespace fts_cs
{

/**
 * @brief This class is used to hold the LUT.
 */
template <class Tk, class Tv>
struct LUT
{
    LUT() = default;
    /**
     * Constructor
     * @param[in] filepath filepath
     */
    LUT(const std::string& filepath)
    {
        load_from_file(filepath);
    }
    virtual ~LUT() = default;

    virtual void load_from_file(const std::string& filepath)
    {
        STDSC_THROW_INVPARAM("not implemented.");
    }
    
    virtual void emplace(const Tk& key, const Tv& val)
    {
        map_.emplace(key, val);
    }

    virtual bool is_exist_key(const Tk& key) const
    {
        return map_.count(key) > 0;
    }

    Tv& operator[] (Tk& key)
    {
        return map_.at(key);
    }
    
    const Tv& operator[] (Tk& key) const
    {
        return map_.at(key);
    }

protected:
    std::unordered_map<Tk, Tv> map_;
};

struct LUTtwo : public LUT<std::string, int64_t>
{
    using super = LUT<std::string, int64_t>;

    LUTtwo() = default;
    virtual ~LUTtwo() = default;

    virtual void load_from_file(const std::string& filepath) override
    {
        // 次回、この関数のテストから
        STDSC_LOG_INFO("Read LUT file. (filepath:%s)", filepath.c_str());

        if (!fts_share::utility::file_exist(filepath)) {
            std::ostringstream oss;
            oss << "File not found. (" << filepath << ")";
            STDSC_THROW_FILE(oss.str());
        }

        std::ifstream ifs(filepath, std::ios::in);
        
        int32_t func = -1;
        size_t  size = -1;
        
        std::string line;
        getline(ifs, line);
        {
            std::string str;
            std::stringstream ss(line);
            
            getline(ss, str, ',');
            if (!fts_share::utility::isdigit(str)) {
                std::ostringstream oss;
                oss << "Invalid format. (filepath:" << filepath << ", function type:" << str << ")";
                STDSC_THROW_FILE(oss.str().c_str());
            }
            func = std::stoi(str);

            getline(ss, str, ',');
            if (!fts_share::utility::isdigit(str)) {
                std::ostringstream oss;
                oss << "Invalid format. (filepath:" << filepath << ", table size:" << str << ")";
                STDSC_THROW_FILE(oss.str().c_str());
            }
            size = std::stoi(str);
        }

        if ((func != 1 && func != 2) || size <= 0) {
            std::ostringstream oss;
            oss << "Invalid format. (filepath:" << filepath;
            oss << ", function type:" << func ;
            oss << ", table size:" << size << ")";
            STDSC_THROW_FILE(oss.str().c_str());
        }

        while (getline(ifs, line)) {
            std::string str;
            std::stringstream ss(line);
            getline(ss, str, ',');
            int64_t x0 = std::stol(str);
            getline(ss, str, ',');
            int64_t x1 = std::stol(str);
            getline(ss, str, ',');
            int64_t y = std::stol(str);
            set(x0, x1, y);
        }

        STDSC_THROW_FILE_IF_CHECK(super::map_.size() < size, "Invalid table size");
    }
    
    void set(const int64_t x0, const int64_t x1, const int64_t y)
    {
        emplace(generate_key(x0, x1), y);
    }
    
    int64_t get(const int64_t x0, const int64_t x1)
    {
        const auto key = generate_key(x0, x1);
        if (!is_exist_key(key)) {
            std::ostringstream oss;
            oss << "Invalid key. Value is not exist in LUT. ";
            oss << "(x0: " << x0 << ", x1: " << x1 << ")";
            STDSC_THROW_INVPARAM(oss.str().c_str());
        }
        return super::map_[key];
    }

private:
    std::string generate_key(const int64_t x0, const int64_t x1) const
    {
        std::ostringstream oss;
        oss << x0 << "," << x1;
        return oss.str();
    }
};

} /* namespace fts_cs */

#endif /* FTS_CS_LUT_HPP */
