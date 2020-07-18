#include <iostream>
#include <cstring>
#include <stdsc/stdsc_buffer.hpp>
#include <stdsc/stdsc_state.hpp>
#include <stdsc/stdsc_socket.hpp>
#include <stdsc/stdsc_packet.hpp>
#include <stdsc/stdsc_exception.hpp>
#include <fts_share/fts_packet.hpp>
#include <fts_dec/fts_dec_srv_callback_function.hpp>
#include <fts_dec/fts_dec_srv_state.hpp>

namespace fts_server
{

DEFUN_REQUEST(CallbackFunctionForNewKeys)
{
    // STDSC_THROW_CALLBACK_IF_CHECK(
    //   kStateReady <= state.current_state(),
    //   "Warn: must be connected state to receive valueB.");
    std::cout << "Received new keys request." << std::endl;
    std::cout << "Called " << __FUNCTION__ << std::endl;
    // DEF_CDATA_ON_EACH(fts_server::CallbackParam);
    // cdata_e->sum = cdata_e->valueA + cdata_e->valueB;
    // state.set(kEventReceivedComputeRequest);
}

DEFUN_DOWNLOAD(CallbackFunctionForResultRequest)
{
    // STDSC_THROW_CALLBACK_IF_CHECK(
    //   kStateComputed <= state.current_state(),
    //   "Warn: must be connected state to receive valueB.");
    std::cout << "Received result request." << std::endl;
    DEF_CDATA_ON_EACH(fts_server::CallbackParam);
    cdata_e->dummy = 0;
    size_t size = sizeof(cdata_e->dummy);
    stdsc::Buffer buffer(size);
    *static_cast<uint32_t*>(buffer.data()) = cdata_e->dummy;
    sock.send_packet(
      stdsc::make_data_packet(fts_share::kControlCodeDataResult, size));
    sock.send_buffer(buffer);
    // state.set(kEventReceivedResultRequest);
}

} /* namespace fts_dec_server */