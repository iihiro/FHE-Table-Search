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

#ifndef FTS_SEAL_UTILITY_HPP
#define FTS_SEAL_UTILITY_HPP

#include <string>
#include <seal/seal.h>

namespace fts_share
{

namespace seal_utility
{
    template <class T>
    void write_to_file(const std::string& filepath, const T& data);
    //{
    //    std::ofstream ofs(filepath, std::ios::binary);
    //    data.save(ofs);
    //    ofs.close();
    //}

    template <>
    void write_to_file<seal::EncryptionParameters>(const std::string& filepath,
                                                   const seal::EncryptionParameters& params);
    //{
    //    std::ofstream ofs(filepath, std::ios::binary);
    //    seal::EncryptionParameters::Save(params, ofs);
    //    ofs.close();
    //}

    template <class T>
    size_t stream_size(const T& data);
    //{
    //    std::ostringstream oss;
    //    data.save(oss);
    //    return oss.str().size();
    //}

    template <>
    size_t stream_size<seal::EncryptionParameters>(const seal::EncryptionParameters& params);
    //{
    //    std::ostringstream oss;
    //    seal::EncryptionParameters::Save(params, oss);
    //    return oss.str().size();
    //}

} /* namespace seal_utility */

} /* namespace fts_share */

#endif /* FTS_SEAL_UTILITY_HPP */
