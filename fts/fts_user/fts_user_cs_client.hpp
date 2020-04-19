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

#ifndef FTS_CLIENT_CS_CLIENT_HPP
#define FTS_CLIENT_CS_CLIENT_HPP

#include <memory>

namespace fts_client
{

/**
 * @brief Provides client for Computation Server.
 */
class CSClient
{
public:
    /**
     * コンストラクタ
     * @param[in] host ComputationServerのホスト名
     * @param[in] port ComputationServerのポート番号
     */
    CSClient(const char* host, const char* port);
    virtual ~CSClient(void) = default;

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
     * クエリ送信
     * @param[in] key_id keyID
     * @param[in] func_no 関数番号
     * @param[in] enc_input 暗号化された入力値(1つ or 2つ)
     * @return queryID
     */
    int32_t send_query(const int32_t key_id, const int32_t func_no, const std::vector<fts_share::EncData>& enc_input) const;
    
    /**
     * 結果受信
     * @param[in] query_id queryID
     * @param[out] enc_result 暗号化された結果
     * @return 結果が受信できたか否か
     */
    bool recv_result(const int32_t query_id, fts_share::EncData& enc_result) const;

private:
    struct Impl;
    std::shared_ptr<Impl> pimpl_;
};

} /* namespace fts_client */

#endif /* FTS_CLIENT_CS_CLIENT_HPP */
