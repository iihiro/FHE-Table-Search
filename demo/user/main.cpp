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
#include <stdsc/stdsc_log.hpp>
#include <stdsc/stdsc_exception.hpp>
#include <fts_share/fts_utility.hpp>
#include <fts_share/fts_pubkey.hpp>
#include <fts_share/fts_seckey.hpp>
#include <fts_user/fts_user_dec_client.hpp>

// static constexpr const char* CONTEXT_FILENAME = "context.txt";
// static constexpr const char* PUBKEY_FILENAME  = "pubkey.txt";


struct Option
{
    uint64_t dummy;
};

void init(Option& option, int argc, char* argv[])
{
}

void exec(Option& option)
{
    const char* host = "localhost";
    
    fts_client::DecClient user_dec_client(host, PORT_DEC_SRV);

    user_dec_client.connect();
    std::cout << "Connected to Server: port["
        << PORT_DEC_SRV << "]" << std::endl;

    fts_share::PubKey pubkey;
    fts_share::SecKey seckey;
    int32_t res_new_keys = user_dec_client.new_keys(pubkey, seckey);
    std::cout << "created new keys: result ["
        << res_new_keys << "]" << std::endl;

    bool res_delete_keys = user_dec_client.delete_keys(0);
    std::cout << "delete keys: result ["
        << res_delete_keys << "]" << std::endl;
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
