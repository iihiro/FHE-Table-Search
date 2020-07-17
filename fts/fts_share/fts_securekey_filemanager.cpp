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

#include <sstream>
#include <memory>
#include <unordered_map>
#include <fstream>
#include <stdsc/stdsc_log.hpp>
#include <stdsc/stdsc_exception.hpp>
#include <fts_share/fts_utility.hpp>
#include <fts_share/fts_securekey_filemanager.hpp>

#define CHECK_KIND(k) do {                                               \
        if (!((k) < kNumOfKind)) {                                       \
            std::ostringstream oss;                                      \
            oss << "Err: Invalid securekey kind. (kind: " << (k) << ")"; \
            STDSC_THROW_INVPARAM(oss.str().c_str());                     \
        }                                                                \
    } while(0)

namespace fts_share
{

struct SecureKeyFileManager::Impl
{
    Impl(const std::string& pubkey_filename,
         const std::string& seckey_filename,
         const std::string& context_filename,
         const std::string& galois_filename,
         const std::string& relin_filename,
         const std::size_t poly_mod_degree,
         const std::size_t coef_mod_192,
         const std::size_t plain_mod)
        : poly_mod_degree_(poly_mod_degree),
          coef_mod_192_(coef_mod_192),
          plain_mod_(plain_mod)
    {
        filenames_.emplace(kKindPubKey,  pubkey_filename);
        filenames_.emplace(kKindSecKey,  seckey_filename);
        filenames_.emplace(kKindContext, context_filename);
        filenames_.emplace(kKindGalois,  galois_filename);
        filenames_.emplace(kKindRelin,   relin_filename);    
    }
    
    ~Impl(void) = default;

    void initialize(void)
    {
        STDSC_LOG_INFO("Generating keys");
    }
    
    size_t size(const Kind_t kind) const
    {
        CHECK_KIND(kind);
        return fts_share::utility::file_size(filename(kind));
    }
    
    void data(const Kind_t kind, void* buffer)
    {
        CHECK_KIND(kind);
    }
    
    bool is_exist(const Kind_t kind) const
    {
        CHECK_KIND(kind);
        std::ifstream ifs(filename(kind));
        return ifs.is_open();
    }
    
    std::string filename(const Kind_t kind) const
    {
        CHECK_KIND(kind);
        return filenames_.at(kind);
    }

private:
    std::unordered_map<Kind_t, std::string> filenames_;
    std::size_t poly_mod_degree_;
    std::size_t coef_mod_192_;
    std::size_t plain_mod_;
};

SecureKeyFileManager::SecureKeyFileManager(const std::string& pubkey_filename,
                                           const std::string& seckey_filename,
                                           const std::string& context_filename,
                                           const std::string& galois_filename,
                                           const std::string& relin_filename,
                                           const std::size_t poly_mod_degree,
                                           const std::size_t coef_mod_192,
                                           const std::size_t plain_mod)
    : pimpl_(new Impl(pubkey_filename, seckey_filename, context_filename,
                      galois_filename, relin_filename,
                      poly_mod_degree,
                      coef_mod_192,
                      plain_mod))
{
}

void SecureKeyFileManager::initialize(void)
{
    pimpl_->initialize();
}

size_t SecureKeyFileManager::size(const Kind_t kind) const
{
    return pimpl_->size(kind);
}

void SecureKeyFileManager::data(const Kind_t kind, void* buffer)
{
    pimpl_->data(kind, buffer);
}

bool SecureKeyFileManager::is_exist(const Kind_t kind) const
{
    return pimpl_->is_exist(kind);
}

std::string SecureKeyFileManager::filename(const Kind_t kind) const
{
    return pimpl_->filename(kind);
}

} /* namespace fts_share */
