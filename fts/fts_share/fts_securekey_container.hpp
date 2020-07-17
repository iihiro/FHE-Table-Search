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

#ifndef FTS_SECUREKEY_CONTAINER
#define FTS_SECUREKEY_CONTAINER

#include <memory>
#include <cstdint>

namespace fts_share
{

class SecureKeyFileManager;

/**
 * @brief Container of securekey file manager
 */
class SecureKeyContainer
{
public:
    SecureKeyContainer();
    ~SecureKeyContainer() = default;

    /// 新規に鍵を生成する
    /// @return キーID
    /// @memo 生成された鍵は内部でテーブル管理する
    int64_t new();

    /// 鍵を取得する
    /// @param[in] keyID キーID
    const SecureKeyFileManager& get(const int64_t keyID) const;
    
private:
    struct Impl;
    std::shared_ptr<Impl> pimpl_;
};
    
} /* namespace fts_share */

#endif /* FTS_SECUREKEY_CONTAINER */
