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
#include <fts_cs/fts_cs_srv_state.hpp>

namespace fts_server
{

StateInit::StateInit()
{
    std::cout << "State: Connected" << std::endl;
}

std::shared_ptr<stdsc::State> StateInit::create()
{
    auto s = std::shared_ptr<stdsc::State>(
      new StateInit());
    return s;
}

void StateInit::set(stdsc::StateContext& sc, uint64_t event)
{
    sc.next_state(StateReady::create());
}

StateReady::StateReady(void)
{
    std::cout << "State: Ready" << std::endl;
}

std::shared_ptr<stdsc::State> StateReady::create()
{
    auto s = std::shared_ptr<stdsc::State>(new StateReady());
    return s;
}

void StateReady::set(stdsc::StateContext& sc, uint64_t event)
{
}


} /* namespace server */
