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

// CallbackFunction for new key Request
DEFUN_DOWNLOAD(CallbackFunctionNewKeyRequest)
{
    STDSC_LOG_INFO("Received new key request. (current state : %s)",
                   state.current_state_str().c_str());

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
    stdsc::Buffer* bsbuff = &buffstream;
    sock.send_packet(stdsc::make_data_packet(fts_share::kControlCodeDataNewKeys, total_sz));
    sock.send_buffer(*bsbuff);
    state.set(kEventNewKeysRequest);
}

// CallbackFunction for Pubkey Request
DEFUN_UPDOWNLOAD(CallbackFunctionPubKeyRequest)
{
    STDSC_LOG_INFO("Received public key request. (current state : %s)",
                   state.current_state_str().c_str());

    DEF_CDATA_ON_ALL(fts_dec::CommonCallbackParam);
    auto& keycont = cdata_a->keycont;

    auto keyID = *static_cast<const int32_t*>(buffer.data());
    STDSC_LOG_INFO("public key request with keyID: %d", keyID);

    auto sz = keycont.size(keyID, KeyKind_t::kKindPubKey);
    stdsc::BufferStream buffstream(sz);
    std::iostream stream(&buffstream);

    seal::PublicKey pubkey;
    keycont.get(keyID, KeyKind_t::kKindPubKey, pubkey);
    pubkey.save(stream);

    STDSC_LOG_INFO("Sending publick key.");
    stdsc::Buffer* bsbuff = &buffstream;
    sock.send_packet(stdsc::make_data_packet(fts_share::kControlCodeDataPubKey, sz));
    sock.send_buffer(*bsbuff);
    state.set(kEventPubKeyRequest);
}

// CallbackFunction for Galoiskey Request
DEFUN_UPDOWNLOAD(CallbackFunctionGaloisKeyRequest)
{
    STDSC_LOG_INFO("Received galois key request. (current state : %s)",
                   state.current_state_str().c_str());

    DEF_CDATA_ON_ALL(fts_dec::CommonCallbackParam);
    auto& keycont = cdata_a->keycont;

    auto keyID = *static_cast<const int32_t*>(buffer.data());
    STDSC_LOG_INFO("galois key request with keyID: %d", keyID);

    auto sz = keycont.size(keyID, KeyKind_t::kKindGaloisKey);
    stdsc::BufferStream buffstream(sz);
    std::iostream stream(&buffstream);

    seal::GaloisKeys galoiskey;
    keycont.get(keyID, KeyKind_t::kKindGaloisKey, galoiskey);
    galoiskey.save(stream);

    STDSC_LOG_INFO("Sending galois key.");
    stdsc::Buffer* bsbuff = &buffstream;
    sock.send_packet(stdsc::make_data_packet(fts_share::kControlCodeDataGaloisKey, sz));
    sock.send_buffer(*bsbuff);
    state.set(kEventGaloisKeyRequest);
}

// CallbackFunction for Relinkey Request
DEFUN_UPDOWNLOAD(CallbackFunctionRelinKeyRequest)
{
    STDSC_LOG_INFO("Received relin key request. (current state : %s)",
                   state.current_state_str().c_str());

    DEF_CDATA_ON_ALL(fts_dec::CommonCallbackParam);
    auto& keycont = cdata_a->keycont;

    auto keyID = *static_cast<const int32_t*>(buffer.data());
    STDSC_LOG_INFO("relin key request with keyID: %d", keyID);

    auto sz = keycont.size(keyID, KeyKind_t::kKindRelinKey);
    stdsc::BufferStream buffstream(sz);
    std::iostream stream(&buffstream);

    seal::RelinKeys relinkey;
    keycont.get(keyID, KeyKind_t::kKindRelinKey, relinkey);
    relinkey.save(stream);

    STDSC_LOG_INFO("Sending relin key.");
    stdsc::Buffer* bsbuff = &buffstream;
    sock.send_packet(stdsc::make_data_packet(fts_share::kControlCodeDataRelinKey, sz));
    sock.send_buffer(*bsbuff);
    state.set(kEventRelinKeyRequest);
}

// CallbackFunction for Param Request
DEFUN_UPDOWNLOAD(CallbackFunctionParamRequest)
{
    STDSC_LOG_INFO("Received param request. (current state : %s)",
                   state.current_state_str().c_str());

    DEF_CDATA_ON_ALL(fts_dec::CommonCallbackParam);
    auto& keycont = cdata_a->keycont;

    auto keyID = *static_cast<const int32_t*>(buffer.data());
    STDSC_LOG_INFO("param request with keyID: %d", keyID);

    auto sz = keycont.size(keyID, KeyKind_t::kKindParam);
    stdsc::BufferStream buffstream(sz);
    std::iostream stream(&buffstream);

    seal::EncryptionParameters params(seal::scheme_type::BFV);
    keycont.get_param(keyID, params);
    seal::EncryptionParameters::Save(params, stream);

    STDSC_LOG_INFO("Sending param.");
    stdsc::Buffer* bsbuff = &buffstream;
    sock.send_packet(stdsc::make_data_packet(fts_share::kControlCodeDataParam, sz));
    sock.send_buffer(*bsbuff);
    state.set(kEventParamRequest);
}

} /* namespace fts_dec_server */
