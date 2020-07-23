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
    auto& qq = cdata_a->query_queue;

    stdsc::BufferStream rbuffstream(buffer);
    std::iostream rstream(&rbuffstream);

    fts_share::PlainData<fts_share::CSParam> rplaindata;
    rplaindata.load_from_stream(rstream);

    seal::EncryptionParameters params(seal::scheme_type::BFV);
    params = seal::EncryptionParameters::Load(rstream);
    
    fts_share::EncData enc_inputs(params);
    enc_inputs.load_from_stream(rstream);
    fts_share::seal_utility::write_to_file("query.txt", enc_inputs.data());

    const auto& param = rplaindata.data();
    STDSC_LOG_INFO("query with key_id: %d, func_no: %d", param.key_id, param.func_no);

    Query query(param.key_id, param.func_no, enc_inputs.vdata());
    int32_t query_id = qq.regist(query);

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

    //DEF_CDATA_ON_ALL(fts_cs::CommonCallbackParam);
    //auto& keycont = cdata_a->keycont;

    auto queryID = *static_cast<const int32_t*>(buffer.data());
    STDSC_LOG_INFO("result request with queryID: %d", queryID);

    fts_share::PlainData<int32_t> plaindata;
    plaindata.push(queryID);
    
    auto sz = plaindata.stream_size();
    stdsc::BufferStream buffstream(sz);
    std::iostream stream(&buffstream);

    plaindata.save_to_stream(stream);

    STDSC_LOG_INFO("Sending result.");
    stdsc::Buffer* bsbuff = &buffstream;
    sock.send_packet(stdsc::make_data_packet(fts_share::kControlCodeDataResult, sz));
    sock.send_buffer(*bsbuff);
    state.set(kEventResultRequest);
}

} /* namespace fts_cs */
