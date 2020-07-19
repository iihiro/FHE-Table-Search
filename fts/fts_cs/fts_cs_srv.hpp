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

#ifndef FTS_CS_HPP
#define FTS_CS_HPP

#define DEFAULT_MAX_CONCURRENT_QUERIES 128
#define DEFAULT_MAX_RESULTS 128
#define DEFAULT_MAX_RESULT_LIFETIME_SEC 50000

#include <memory>

namespace fts_server
{

/**
 * @brief Provides Computation Server.
 */
class CSServer
{
public:
    /**
     * コンストラクタ
     * @param[in] port ポート番号
     * @param[in] LUT_dirpath LUTファイルのディレクトリパス
     * @param[in] callback コールバック関数定義
     * @param[in] state 状態遷移定義
     * @param[in] max_concurrent_queries 最大同時クエリー数
     * @param[in] max_results 最大結果保持数
     * @param[in] result_lifetime_sec 結果を保持する時間(秒)
     */
    CSServer(const char* port,
             const std::string& LUT_dirpath,
             stdsc::CallbackFunctionContainer& callback,
             stdsc::StateContext& state,
             const uint32_t max_concurrent_queries = DEFAULT_MAX_CONCURRENT_QUERIES,
             const uint32_t max_results = DEFAULT_MAX_RESULTS,
             const uint32_t result_lifetime_sec = DEFAULT_MAX_RESULT_LIFETIME_SEC);
    ~CSServer(void) = default;

    /**
     * サーバ実行
     */
    void start(void);
    /**
     * サーバ停止指示
     */
    void stop(void);
    /**
     * サーバ停止待ち
     */
    void wait(void);

private:
    struct Impl;
    std::shared_ptr<Impl> pimpl_;
};

} /* namespace ftscs */

#endif /* FTS_CS_SRV_HPP */
