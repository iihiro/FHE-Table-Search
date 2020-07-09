#include <iostream>
#include <fts_dec/fts_dec_srv_state.hpp>


namespace fts_dec_server
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
}

} /* namespace fts_dec_server */
