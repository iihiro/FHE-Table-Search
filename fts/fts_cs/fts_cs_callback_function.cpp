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

#include <iostream>
#include <cstring>
#include <stdsc/stdsc_buffer.hpp>
#include <stdsc/stdsc_state.hpp>
#include <stdsc/stdsc_socket.hpp>
#include <stdsc/stdsc_packet.hpp>
#include <stdsc/stdsc_exception.hpp>
#include <fts_share/fts_packet.hpp>
#include <fts_share/fts_plaindata.hpp>
#include <fts_share/fts_encdata.hpp>
#include <fts_share/fts_csparam.hpp>
#include <fts_share/fts_seal_utility.hpp>
#include <fts_cs/fts_cs_callback_function.hpp>
#include <fts_cs/fts_cs_callback_param.hpp>
#include <fts_cs/fts_cs_query.hpp>
#include <fts_cs/fts_cs_result.hpp>
#include <fts_cs/fts_cs_calcmanager.hpp>
#include <fts_cs/fts_cs_state.hpp>

#include <seal/seal.h>

namespace fts_cs
{

// CallbackFunction for Query
DEFUN_UPDOWNLOAD(CallbackFunctionQuery)
{
    STDSC_LOG_INFO("Received query. (current state : %s)",
                   state.current_state_str().c_str());

    DEF_CDATA_ON_ALL(fts_cs::CommonCallbackParam);
    auto& calc_manager = cdata_a->calc_manager_;

    stdsc::BufferStream rbuffstream(buffer);
    std::iostream rstream(&rbuffstream);

    // load plaindata (param)
    fts_share::PlainData<fts_share::CSParam> rplaindata;
    rplaindata.load_from_stream(rstream);
    const auto& param = rplaindata.data();
    STDSC_LOG_INFO("query with key_id: %d, func_no: %d", param.key_id, param.func_no);

    // load encryption parameters
    seal::EncryptionParameters params(seal::scheme_type::BFV);
    params = seal::EncryptionParameters::Load(rstream);

    // load encryption inputs
    fts_share::EncData enc_inputs(params);
    enc_inputs.load_from_stream(rstream);
    fts_share::seal_utility::write_to_file("query.txt", enc_inputs.data());

    printf("a\n");
    Query query(param.key_id, param.func_no, enc_inputs.vdata());
    int32_t query_id = calc_manager.push_query(query);

    fts_share::PlainData<int32_t> splaindata;
    splaindata.push(query_id);

    auto sz = splaindata.stream_size();
    stdsc::BufferStream sbuffstream(sz);
    std::iostream sstream(&sbuffstream);

    splaindata.save_to_stream(sstream);

    STDSC_LOG_INFO("Sending queryID.");
    stdsc::Buffer* bsbuff = &sbuffstream;
    sock.send_packet(stdsc::make_data_packet(fts_share::kControlCodeDataQueryID, sz));
    sock.send_buffer(*bsbuff);
    state.set(kEventQuery);
}

// CallbackFunction for Result Request
DEFUN_UPDOWNLOAD(CallbackFunctionResultRequest)
{
    STDSC_LOG_INFO("Received result request. (current state : %s)",
                   state.current_state_str().c_str());

    DEF_CDATA_ON_ALL(fts_cs::CommonCallbackParam);
    auto& calc_manager = cdata_a->calc_manager_;

    stdsc::BufferStream rbuffstream(buffer);
    std::iostream rstream(&rbuffstream);

    // load plaindata (param)
    fts_share::PlainData<int32_t> rplaindata;
    rplaindata.load_from_stream(rstream);
    const auto query_id = rplaindata.data();
    STDSC_LOG_INFO("result request with query_id: %d", query_id);

    // load encryption parameters
    seal::EncryptionParameters params(seal::scheme_type::BFV);
    params = seal::EncryptionParameters::Load(rstream);

    Result result;
    calc_manager.pop_result(query_id, result);

    fts_share::EncData enc_outputs(params, result.ctxt_);
    fts_share::seal_utility::write_to_file("result.txt", enc_outputs.data());
    
    auto sz = enc_outputs.stream_size();
    printf("hoge: sz:%lu\n", sz);
    stdsc::BufferStream sbuffstream(sz);
    std::iostream sstream(&sbuffstream);

    enc_outputs.save_to_stream(sstream);

    STDSC_LOG_INFO("Sending result.");
    stdsc::Buffer* bsbuff = &sbuffstream;
    sock.send_packet(stdsc::make_data_packet(fts_share::kControlCodeDataResult, sz));
    sock.send_buffer(*bsbuff);
    state.set(kEventResultRequest);
}

} /* namespace fts_cs */
