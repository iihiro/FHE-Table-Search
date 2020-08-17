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
#include <iostream>

namespace fts_cs
{

/**
 * @brief Enumeration for state.
 */
enum LUTFunc_t : int32_t
{
    kLUTFuncNil       = 0,
    kLUTFuncLinear    = 1,
    kLUTFuncQuadratic = 2,
};
    
/**
 * Get function number from file
 * @param[in] filepath
 * @return function number
 */
LUTFunc_t fts_cs_lut_get_funcnumber(const std::string& filepath);
    
/**
 * Read header
 * @param[in/out] ifs input file stream
 * @param[out] func function number
 * @param[out] size table size
 * @memo
 *   Header format
 *   -------------
 *   func, size
 *   -------------
 *
 *   - func : LUTFunc_t
 *   - size : table col size (without header)
 */
void fts_cs_lut_read_header(std::ifstream& ifs, LUTFunc_t& func, size_t& size);
    
/**
 * @brief This class is used to hold the LUT.
 */
template <class Tk, class Tv>
struct LUTBase
{
    LUTBase() = default;
    virtual ~LUTBase() = default;

    /**
     * Load from file
     * @param[in] filepath filepath
     */
    virtual void load_from_file(const std::string& filepath)
    {
        STDSC_THROW_INVPARAM("not implemented.");
    }

    /**
     * Emplace key-value
     * @param[in] key key
     * @parma[in] val value
     */
    virtual void emplace(const Tk& key, const Tv& val)
    {
        map_.emplace(key, val);
    }

    /**
     * Check key
     * @param[in] key key
     * @return exists key
     */
    virtual bool is_exist_key(const Tk& key) const
    {
        return map_.count(key) > 0;
    }

    /**
     * Get values with specified key
     * @param[in] key key
     * @return value
     */
    Tv& operator[] (Tk& key)
    {
        return map_.at(key);
    }
    
    /**
     * Get values with specified keys
     * @param[in] key key
     * @return value
     */
    const Tv& operator[] (Tk& key) const
    {
        return map_.at(key);
    }
    
    /**
     * Dump map_
     */
    void dump() const
    {
        std::cout << "Dump LUT: " << std::endl;
        for (const auto& pair : map_) {
            std::cout << "  " << pair.first << ", " << pair.second << std::endl;
        }
    }

public:
    /**
     * Get function number from file
     * @param[in] filepath
     * @return function number
     */
    static int32_t get_function_number(const std::string& filepath);
    
protected:
    /**
     * Read header
     * @param[in/out] ifs input file stream
     * @param[out] func function number
     * @param[out] size table size
     * @memo
     *   Header format
     *   -------------
     *   func, size
     *   -------------
     *
     *   - func : 1: linear function, 2: quadratic finction
     *   - size : table col size (without header)
     */
    void read_header(std::ifstream& ifs, int32_t& func, size_t& size) const;

protected:
    std::unordered_map<Tk, Tv> map_;
};

    
/**
 * @brief This class is used to hold the LUT of linear function.
 */
struct LUTLFunc : public LUTBase<std::string, int64_t>
{
    using super = LUTBase<std::string, int64_t>;

    LUTLFunc() = default;
    /**
     * Constructor
     * @param[in] filepath filepath
     */
    LUTLFunc(const std::string& filepath);
    virtual ~LUTLFunc() = default;

    /**
     * Load from file
     * @param[in] filepath filepath
     */
    virtual void load_from_file(const std::string& filepath) override;
    
    /**
     * Set values with specified keys
     * @param[in] x x
     * @param[in] y
     */
    void set(const int64_t x, const int64_t y);
    
    /**
     * Get values with specified keys
     * @param[in] x x
     * @return y
     */
    int64_t get(const int64_t x);

private:
    /**
     * Generate key strings
     * @param[in] x0 x0
     * @param[in] x1 x1
     * @return key strings
     */
    std::string generate_key(const int64_t x) const;
    
};

    
/**
 * @brief This class is used to hold the LUT of quadratic function.
 */
struct LUTQFunc : public LUTBase<std::string, int64_t>
{
    using super = LUTBase<std::string, int64_t>;

    LUTQFunc() = default;
    /**
     * Constructor
     * @param[in] filepath filepath
     */
    LUTQFunc(const std::string& filepath);
    virtual ~LUTQFunc() = default;

    /**
     * Load from file
     * @param[in] filepath filepath
     */
    virtual void load_from_file(const std::string& filepath) override;
    
    /**
     * Set values with specified keys
     * @param[in] x0 x0
     * @param[in] x1 x1
     * @param[in] y
     */
    void set(const int64_t x0, const int64_t x1, const int64_t y);
    
    /**
     * Get values with specified keys
     * @param[in] x0 x0
     * @param[in] x1 x1
     * @return y
     */
    int64_t get(const int64_t x0, const int64_t x1);

private:
    
    /**
     * Generate key strings
     * @param[in] x0 x0
     * @param[in] x1 x1
     * @return key strings
     */
    std::string generate_key(const int64_t x0, const int64_t x1) const;
};
    

} /* namespace fts_cs */

#endif /* FTS_CS_LUT_HPP */
