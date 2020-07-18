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

#ifndef FTS_DEC_SRV_CALLBACK_FUNCTION_HPP
#define FTS_DEC_SRV_CALLBACK_FUNCTION_HPP

#include <stdsc/stdsc_callback_function.hpp>

namespace fts_dec
{

/**
 * @brief Provides callback function in receiving new key request.
 */
DECLARE_DOWNLOAD_CLASS(CallbackFunctionNewKeyRequest);

///**
// * @brief Provides callback function in receiving pubic key request.
// */
//DECLARE_DOWNLOAD_CLASS(CallbackFunctionPubkeyRequest);
//
///**
// * @brief Provides callback function in receiving galois key request.
// */
//DECLARE_DOWNLOAD_CLASS(CallbackFunctionGaloisKeyRequest);
//
///**
// * @brief Provides callback function in receiving Relin key request.
// */
//DECLARE_DOWNLOAD_CLASS(CallbackFunctionRelinKeyRequest);
//    
///**
// * @brief Provides callback function in receiving context request.
// */
//DECLARE_DOWNLOAD_CLASS(CallbackFunctionContextRequest);
//    
///**
// * @brief Provides callback function in receiving pamameter request.
// */
//DECLARE_DOWNLOAD_CLASS(CallbackFunctionParamRequest);


} /* namespace fts_dec */

#endif /* FTS_DEC_SRV_CALLBACK_FUNCTION_HPP */
