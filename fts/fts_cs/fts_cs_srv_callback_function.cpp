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
#include <fts_cs/fts_cs_srv_callback_function.hpp>
#include <fts_cs/fts_cs_srv_state.hpp>

namespace fts_server
{

DEFUN_REQUEST(CallbackFunctionForComputeRequest)
{
    // STDSC_THROW_CALLBACK_IF_CHECK(
    //   kStateReady <= state.current_state(),
    //   "Warn: must be connected state to receive query_id.");
    std::cout << "Received compute request." << std::endl;
    std::cout << "Called " << __FUNCTION__ << std::endl;
    DEF_CDATA_ON_EACH(fts_server::CallbackParam);
    cdata_e->query_id = 1111;
    state.set(kEventReceivedQuery);
}

DEFUN_DOWNLOAD(CallbackFunctionForQueryID)
{
    // STDSC_THROW_CALLBACK_IF_CHECK(
    //   kStateComputed <= state.current_state(),
    //   "Warn: must be connected state to receive query_id.");
    std::cout << "Received query request." << std::endl;
    DEF_CDATA_ON_EACH(fts_server::CallbackParam);
    size_t size = sizeof(cdata_e->query_id);
    stdsc::Buffer buffer(size);
    *static_cast<uint32_t*>(buffer.data()) = cdata_e->query_id;
    sock.send_packet(
      stdsc::make_data_packet(fts_share::kControlCodeDataResult, size));
    sock.send_buffer(buffer);
    state.set(kEventReceivedQuery);
}

DEFUN_DOWNLOAD(CallbackFunctionForResultRequest)
{
    // STDSC_THROW_CALLBACK_IF_CHECK(
    //   kStateComputed <= state.current_state(),
    //   "Warn: must be connected state to receive enc_results.");
    std::cout << "Received result request." << std::endl;
    DEF_CDATA_ON_EACH(fts_server::CallbackParam);
    cdata_e->enc_results = 0;
    size_t size = sizeof(cdata_e->enc_results);
    stdsc::Buffer buffer(size);
    *static_cast<uint32_t*>(buffer.data()) = cdata_e->enc_results;
    sock.send_packet(
      stdsc::make_data_packet(fts_share::kControlCodeDataResult, size));
    sock.send_buffer(buffer);
    state.set(kEventReceivedResultRequest);
}

} /* namespace fts_server */
