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

#ifndef FTS_CLIENT_DEC_CLIENT_HPP
#define FTS_CLIENT_DEC_CLIENT_HPP

#include <memory>

namespace fts_client
{

/**
 * @brief Provides client for Decryptor.
 */
class DECClient
{
public:
    /**
     * コンストラクタ
     * @param[in] host Decryptorのホスト名
     * @param[in] port Decryptorのポート番号
     */
    DECClient(const char* host, const char* port);
    virtual ~DECClient(void) = default;

    /**
     * 接続
     * @param[in] retry_interval_usec リトライ間隔(usec)
     * @param[in] timeout_sec タイムアウト時間(sec)
     */
    void connect(const uint32_t retry_interval_usec = FTS_RETRY_INTERVAL_USEC,
                 const uint32_t timeout_sec = FTS_TIMEOUT_SEC);
    /**
     * 切断
     */
    void disconnect();

    /**
     * 新規鍵ペア生成
     * @return keyID
     */
    int32_t new_keys();
    /**
     * 鍵ペア削除
     * @param[in] key_id keyID
     * @return 処理に成功したか否か
     */
    bool delete_keys(const int32_t key_id) const;

private:
    struct Impl;
    std::shared_ptr<Impl> pimpl_;
};

} /* namespace fts_client */

#endif /* FTS_CLIENT_DEC_CLIENT_HPP */
