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

#ifndef FTS_SECUREKEY_FILEMANAGER
#define FTS_SECUREKEY_FILEMANAGER

#include <memory>
#include <fts_share/fts_decparam.hpp>

namespace fts_share
{

/**
 * @brief Manages secure key files. (public / secret key files)
 */
class SecureKeyFileManager
{
public:

    enum Kind_t : int32_t
    {
        kKindUnknown = -1,
        kKindPubKey  = 0,
        kKindSecKey  = 1,
        kKindContext = 2,
        kKindGalois  = 3,
        kKindRelin   = 4,
        kNumOfKind,
    };
    
public:
    SecureKeyFileManager(const std::string& pubkey_filename,
                         const std::string& seckey_filename,
                         const std::string& context_filename,
                         const std::string& galois_filename,
                         const std::string& relin_filename,
                         const std::size_t poly_mod_degree = DecParam::DefaultPolyModDegree,
                         const std::size_t coef_mod_192    = DecParam::DefaultCoefMod192,
                         const std::size_t plain_mod       = DecParam::DefaultPlainMod);

    ~SecureKeyFileManager(void) = default;

    void initialize(void);

    size_t size(const Kind_t kind) const;
    
    void data(const Kind_t kind, void* buffer);
    
    bool is_exist(const Kind_t kind) const;
    
    std::string filename(const Kind_t kind) const;

private:
    struct Impl;
    std::shared_ptr<Impl> pimpl_;
};
    
} /* namespace fts_share */

#endif /* FTS_SECUREKEY_FILEMANAGER */
