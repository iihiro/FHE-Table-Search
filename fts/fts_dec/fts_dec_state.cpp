#include <iostream>
#include <fts_dec/fts_dec_srv_state.hpp>


namespace fts_dec
{

struct StateReady::Impl
{
    Impl(void)
    {
    }

    void set(stdsc::StateContext& sc, uint64_t event)
    {
        STDSC_LOG_TRACE("StateReady: event#%lu", event);
        switch (static_cast<Event_t>(event))
        {
            default:
                break;
        }
    }
};

// Ready

std::shared_ptr<stdsc::State> StateReady::create(void)
{
    return std::shared_ptr<stdsc::State>(new StateReady());
}

StateReady::StateReady()
  : pimpl_(new Impl())
{
}

} /* namespace fts_dec */
