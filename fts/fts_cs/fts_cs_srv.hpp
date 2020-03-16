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

#include <memory>

namespace ftscs
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
     * @param[in] LUT_filepath LUTファイルパス
     * @param[in] callback コールバック関数定義
     * @param[in] state 状態遷移定義
     * @param[in] max_concurrent_query 最大同時クエリー数
     */
    CSServer(const char* port,
             const std::string& LUT_filepath,
             stdsc::CallbackFunctionContainer& callback,
             stdsc::StateContext& state,
             const uint32_t max_concurrent_query = DEFAULT_MAX_CONCURRENT_QUERY);
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
