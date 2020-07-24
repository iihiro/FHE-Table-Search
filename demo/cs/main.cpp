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

#include <memory>
#include <string>
#include <iostream>
#include <share/define.hpp>
#include <stdsc/stdsc_state.hpp>
#include <stdsc/stdsc_callback_function.hpp>
#include <stdsc/stdsc_callback_function_container.hpp>
#include <stdsc/stdsc_log.hpp>
#include <stdsc/stdsc_exception.hpp>
#include <fts_share/fts_utility.hpp>
#include <fts_share/fts_packet.hpp>
#include <fts_cs/fts_cs_srv.hpp>
#include <fts_cs/fts_cs_state.hpp>
#include <fts_cs/fts_cs_callback_param.hpp>
#include <fts_cs/fts_cs_callback_function.hpp>

// static constexpr const char* CONTEXT_FILENAME = "context.txt";
// static constexpr const char* PUBKEY_FILENAME  = "pubkey.txt";
// static constexpr const char* SECKEY_FILENAME  = "seckey.txt";


struct Option
{
    uint64_t dummy;
};

void init(Option& option, int argc, char* argv[])
{
}

void exec(Option& option)
{
    stdsc::StateContext state(std::make_shared<fts_cs::StateReady>());
    
    stdsc::CallbackFunctionContainer callback;
    {
        std::shared_ptr<stdsc::CallbackFunction> cb_query(
            new fts_cs::CallbackFunctionQuery());
        callback.set(fts_share::kControlCodeUpDownloadQuery, cb_query);

        std::shared_ptr<stdsc::CallbackFunction> cb_result(
            new fts_cs::CallbackFunctionResultRequest());
        callback.set(fts_share::kControlCodeUpDownloadResult, cb_result);
    }

    const std::string LUT_dirpath = "hoge";
    const char* dec_host = "localhost";

    std::shared_ptr<fts_cs::CSServer> cs_server
        (new fts_cs::CSServer(PORT_CS_SRV, dec_host, PORT_DEC_SRV, LUT_dirpath, callback, state));

    cs_server->start();
    
    std::string key;
    std::cout << "hit any key to exit server: " << std::endl;
    std::cin >> key;

    cs_server->stop();
    cs_server->wait();
}

int main(int argc, char* argv[])
{
    STDSC_INIT_LOG();
    try
    {
        Option option;
        init(option, argc, argv);
        STDSC_LOG_INFO("Launched User demo app.");
        exec(option);
    }
    catch (stdsc::AbstractException& e)
    {
        STDSC_LOG_ERR("Err: %s", e.what());
    }
    catch (...)
    {
        STDSC_LOG_ERR("Catch unknown exception");
    }

    return 0;
}
