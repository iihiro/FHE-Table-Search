#include <iostream>
#include <cstring>
#include <stdsc/stdsc_buffer.hpp>
#include <stdsc/stdsc_state.hpp>
#include <stdsc/stdsc_socket.hpp>
#include <stdsc/stdsc_packet.hpp>
#include <stdsc/stdsc_exception.hpp>
#include <fts_share/fts_packet.hpp>
#include <fts_share/fts_plaindata.hpp>
#include <fts_dec/fts_dec_callback_function.hpp>
#include <fts_dec/fts_dec_callback_param.hpp>
#include <fts_dec/fts_dec_keycontainer.hpp>
#include <fts_dec/fts_dec_state.hpp>

namespace fts_dec
{

DEFUN_DOWNLOAD(CallbackFunctionNewKeyRequest)
{
    std::cout << "Received new key request." << std::endl;
    DEF_CDATA_ON_EACH(fts_dec::CallbackParam);
    DEF_CDATA_ON_ALL(fts_dec::CommonCallbackParam);
    auto& param   = cdata_e->param;
    auto& keycont = cdata_a->keycont;

    auto keyID = keycont.new_keys(param);

    fts_share::PlainData<int32_t> plaindata;
    plaindata.push(keyID);

    auto seckey_sz = keycont.size(keyID, KeyKind_t::kKindSecKey);
    auto keyID_sz  = plaindata.stream_size();
    auto total_sz  = seckey_sz + keyID_sz;
    
    stdsc::BufferStream buffstream(total_sz);
    std::iostream stream(&buffstream);

    seal::SecretKey seckey;
    keycont.get(keyID, KeyKind_t::kKindSecKey, seckey);
    seckey.save(stream);

    plaindata.save_to_stream(stream);
    
    STDSC_LOG_INFO("Sending keyID and secret key.");
    stdsc::Buffer* buffer = &buffstream;
    sock.send_packet(stdsc::make_data_packet(fts_share::kControlCodeDataNewKeys, total_sz));
    sock.send_buffer(*buffer);
    state.set(kEventNewKeysRequest);
}

//// CallbackFunctionPubkeyRequest
//DEFUN_DOWNLOAD(CallbackFunctionPubkeyRequest)
//{
//    STDSC_LOG_INFO("Received public key request. (current state : %s)",
//                   state.current_state_str().c_str());
//
//    DEF_CDATA_ON_EACH(fts_dec::CallbackParam);
//
//    auto  kind = fts_share::SecureKeyFileManager::Kind_t::kKindPubKey;
//    auto& skm  = *cdata_e->skm_ptr;
//    stdsc::Buffer pubkey(skm.size(kind));
//    skm.data(kind, pubkey.data());
//    STDSC_LOG_INFO("Sending public key.");
//    sock.send_packet(stdsc::make_data_packet(fts_share::kControlCodeDataPubkey,
//                                             skm.size(kind)));
//    sock.send_buffer(pubkey);
//    state.set(kEventPubkeyRequest);
//}

//// CallbackFunctionContextRequest
//DEFUN_DOWNLOAD(CallbackFunctionContextRequest)
//{
//    STDSC_LOG_INFO("Received context request. (current state : %s)",
//                   state.current_state_str().c_str());
//
//    DEF_CDATA_ON_EACH(fts_dec::CallbackParam);
//
//    auto  kind = fts_share::SecureKeyFileManager::Kind_t::kKindContext;
//    auto& skm  = *cdata_e->skm_ptr;
//    stdsc::Buffer context(skm.size(kind));
//    skm.data(kind, context.data());
//    STDSC_LOG_INFO("Sending context.");
//    sock.send_packet(stdsc::make_data_packet(fts_share::kControlCodeDataContext,
//                                             skm.size(kind)));
//    sock.send_buffer(context);
//    state.set(kEventContextRequest);
//}
//
//// CallbackFunctionEMKRequest
//DEFUN_DOWNLOAD(CallbackFunctionEMKRequest)
//{
//    STDSC_LOG_INFO("Received emk request. (current state : %s)",
//                   state.current_state_str().c_str());
//
//    DEF_CDATA_ON_EACH(fts_dec::CallbackParam);
//
//    auto  kind = fts_share::SecureKeyFileManager::Kind_t::kKindEMK;
//    auto& skm  = *cdata_e->skm_ptr;
//    stdsc::Buffer emk(skm.size(kind));
//    skm.data(kind, emk.data());
//    STDSC_LOG_INFO("Sending emk.");
//    sock.send_packet(stdsc::make_data_packet(fts_share::kControlCodeDataEMK,
//                                             skm.size(kind)));
//    sock.send_buffer(emk);
//    state.set(kEventEMKRequest);
//}
//
//// CallbackFunctionEAKRequest
//DEFUN_DOWNLOAD(CallbackFunctionEAKRequest)
//{
//    STDSC_LOG_INFO("Received eak request. (current state : %s)",
//                   state.current_state_str().c_str());
//
//    DEF_CDATA_ON_EACH(fts_dec::CallbackParam);
//
//    auto  kind = fts_share::SecureKeyFileManager::Kind_t::kKindEAK;
//    auto& skm  = *cdata_e->skm_ptr;
//    stdsc::Buffer eak(skm.size(kind));
//    skm.data(kind, eak.data());
//    STDSC_LOG_INFO("Sending eak.");
//    sock.send_packet(stdsc::make_data_packet(fts_share::kControlCodeDataEAK,
//                                             skm.size(kind)));
//    sock.send_buffer(eak);
//    state.set(kEventEAKRequest);
//}
//
//// CallbackFunctionParamRequest
//DEFUN_DOWNLOAD(CallbackFunctionParamRequest)
//{
//    STDSC_LOG_INFO("Received param request. (current state : %s)",
//                   state.current_state_str().c_str());
//
//    DEF_CDATA_ON_EACH(fts_dec::CallbackParam);
//
//    fts_share::PlainData<fts_share::DecParam> plaindata;
//    plaindata.push(cdata_e->param);
//
//    auto sz = plaindata.stream_size();
//    stdsc::BufferStream buffstream(sz);
//    std::iostream stream(&buffstream);
//
//    plaindata.save_to_stream(stream);
//
//    STDSC_LOG_INFO("Sending param.");
//    stdsc::Buffer* buffer = &buffstream;
//    sock.send_packet(stdsc::make_data_packet(fts_share::kControlCodeDataDecParam, sz));
//    sock.send_buffer(*buffer);
//    state.set(kEventParamRequest);
//}


} /* namespace fts_dec_server */
