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
#include <unistd.h>
#include <share/define.hpp>
#include <stdsc/stdsc_state.hpp>
#include <stdsc/stdsc_log.hpp>
#include <stdsc/stdsc_exception.hpp>
#include <stdsc/stdsc_utility.hpp>
#include <stdsc/stdsc_buffer.hpp>
#include <fts_share/fts_utility.hpp>
#include <fts_share/fts_seal_utility.hpp>
#include <fts_share/fts_csparam.hpp>
#include <fts_share/fts_encdata.hpp>
#include <fts_user/fts_user_dec_client.hpp>
#include <fts_user/fts_user_cs_client.hpp>
#include <fts_user/fts_user_result_thread.hpp>


#define PRINT_USAGE_AND_EXIT() do {                         \
        printf("Usage: %s value_x [value_y]\n", argv[0]);   \
        exit(1);                                            \
    } while (0)

seal::SecretKey g_seckey;

struct Option
{
    int64_t input_value_x;
    int64_t input_value_y;
    int32_t input_num = 0;
};

struct CallbackParam
{
    int32_t dummy = 222;
};

void result_cb(const seal::Ciphertext& enc_results, void* args)
{
    printf("called result callback.");
}

void init(Option& option, int argc, char* argv[])
{
    if (argc < 2) {
        PRINT_USAGE_AND_EXIT();
    } else {
        if (stdsc::utility::isdigit(argv[1])) {
            option.input_value_x = std::stol(argv[1]);
            option.input_num++;
        }
    }

    if (argc >= 3) {
        if (stdsc::utility::isdigit(argv[2])) {
            option.input_value_y = std::stol(argv[2]);
            option.input_num++;
        }
    } 
}

int32_t init_keys(const std::string& dec_host,
                  const std::string& dec_port,
                  seal::SecretKey& seckey,
                  seal::PublicKey& pubkey,
                  seal::GaloisKeys& galoiskey,
                  seal::EncryptionParameters& params)
{
    fts_user::DecClient dec_client(dec_host.c_str(), dec_port.c_str());
    dec_client.connect();
    
    auto key_id = dec_client.new_keys(seckey);
    STDSC_LOG_INFO("generate new keys. (key_id:%d)", key_id);
    fts_share::seal_utility::write_to_file("seckey.txt", seckey);

    dec_client.get_pubkey(key_id, pubkey);
    fts_share::seal_utility::write_to_file("pubkey.txt", pubkey);

    dec_client.get_galoiskey(key_id, galoiskey);
    fts_share::seal_utility::write_to_file("galoiskey.txt", galoiskey);

    seal::RelinKeys relinkey;
    dec_client.get_relinkey(key_id, relinkey);
    fts_share::seal_utility::write_to_file("relinkey.txt", relinkey);
        
    dec_client.get_param(key_id, params);
    fts_share::seal_utility::write_to_file("param.txt", params);
    
    return key_id;
}

void compute_one(const int32_t key_id,
                 const int64_t val,
                 const std::string& cs_host,
                 const std::string& cs_port,
                 const seal::PublicKey& pubkey,
                 const seal::GaloisKeys& galoiskey,
                 const seal::EncryptionParameters& params)
{
    fts_share::EncData enc_inputs(params);
    enc_inputs.encrypt(val, pubkey, galoiskey);

    // experiment
    // この実験ができたので、次回はCSへクエリを送って、それを受け取るCallbackを各ところから
    {
        // save to file
        fts_share::seal_utility::write_to_file("query.txt", enc_inputs.data());
            
        // save to stream
        auto sz = enc_inputs.stream_size();
        stdsc::BufferStream buffstream(sz);
        std::iostream stream(&buffstream);
        enc_inputs.save_to_stream(stream);

        // load from stream
        fts_share::EncData enc_inputs2(params);
        enc_inputs2.load_from_stream(stream);

        // decrypt
        int64_t output_value;
        enc_inputs2.decrypt(g_seckey, output_value);
        printf("  -- %ld\n", output_value);
    }
        
    fts_user::CSClient cs_client(cs_host.c_str(), cs_port.c_str(), params);
    cs_client.connect();

    CallbackParam result_cbargs;
    auto query_id = cs_client.send_query(key_id, fts_share::kFuncOne, enc_inputs,
                                         result_cb, &result_cbargs);
    printf("query_id: %d\n", query_id);

    usleep(5*1000*1000);
}

void exec(Option& option)
{
    const char* host = "localhost";

    seal::PublicKey pubkey;
    seal::GaloisKeys galoiskey;
    seal::EncryptionParameters params(seal::scheme_type::BFV);
    auto key_id = init_keys(host, PORT_DEC_SRV,
                            g_seckey, pubkey, galoiskey, params);

    if (option.input_num == 1) {
        compute_one(key_id, option.input_value_x, host, PORT_CS_SRV, pubkey, galoiskey, params);
    }

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
