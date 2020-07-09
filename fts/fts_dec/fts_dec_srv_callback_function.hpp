#ifndef FTS_DEC_SRV_CALLBACK_FUNCTION_HPP
#define FTS_DEC_SRV_CALLBACK_FUNCTION_HPP

#include <string>
#include <cstdint>
#include <stdsc/stdsc_callback_function.hpp>

namespace fts_dec_server
{

struct CallbackParam
{
    int32_t dummy;
};

DECLARE_REQUEST_CLASS(CallbackFunctionForNewKeys);
DECLARE_REQUEST_CLASS(CallbackFunctionForDeleteKeys);
DECLARE_DOWNLOAD_CLASS(CallbackFunctionForResultRequest);

} /* namespace fts_dec_server */

#endif /* FTS_DEC_SRV_CALLBACK_FUNCTION_HPP */