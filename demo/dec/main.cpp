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
#include <fts_dec/fts_dec_srv.hpp>
#include <fts_dec/fts_dec_srv_state.hpp>
#include <fts_dec/fts_dec_srv_callback_function.hpp>

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
    stdsc::StateContext state(std::make_shared<fts_server::StateInit>());
    
    stdsc::CallbackFunctionContainer callback;
    fts_server::CallbackParam param;
    {
        std::shared_ptr<stdsc::CallbackFunction> cb_new_keys(
            new fts_server::CallbackFunctionForNewKeys()
        );
        callback.set(fts_share::kControlCodeRequestNewKeys, cb_new_keys);

        std::shared_ptr<stdsc::CallbackFunction> cb_result(
            new fts_server::CallbackFunctionForResultRequest()
        );
        callback.set(fts_share::kControlCodeDownloadResult, cb_result);
    }
    callback.set_commondata(static_cast<void*>(&param), sizeof(param));

    std::shared_ptr<fts_server::DecServer> dec_server
        (new fts_server::DecServer(PORT_DEC_SRV, callback, state));

    dec_server->start();
    
    std::string key;
    std::cout << "hit any key to exit server: " << std::endl;
    std::cin >> key;

    dec_server->stop();
    dec_server->wait();
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
