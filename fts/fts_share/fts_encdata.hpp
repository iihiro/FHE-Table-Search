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

#ifndef FTS_ENCDATA_HPP
#define FTS_ENCDATA_HPP

#include <memory>
#include <fts_share/fts_basicdata.hpp>
#include <seal/seal.h>

namespace fts_share
{

class PubKey;
class SecKey;
class Context;
    
/**
 * @brief This class is used to hold the encrypted data.
 */
    struct EncData : public fts_share::BasicData<seal::Ciphertext>
{
    explicit EncData(const seal::SEALContext& context);
    virtual ~EncData(void) = default;

    void encrypt(const std::vector<int64_t>& inputdata, const PubKey& pubkey);
    void decrypt(const SecKey& seckey, std::vector<int64_t>& outputdata) const;

    virtual void save_to_stream(std::ostream& os) const override;
    virtual void load_from_stream(std::istream& is) override;

    void save_to_file(const std::string& filepath) const;
    void load_from_file(const std::string& filepath);


private:
    struct Impl;
    std::shared_ptr<Impl> pimpl_;
};

} /* namespace fts_share */

#endif /* FTS_ENCDATA_HPP */
