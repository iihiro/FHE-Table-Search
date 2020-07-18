#ifndef FTS_DEC_SRV_STATE_HPP
#define FTS_DEC_SRV_STATE_HPP

#include <memory>
#include <cstdbool>
#include <stdsc/stdsc_state.hpp>

namespace fts_dec
{

/**
 * @brief Enumeration for state.
 */
enum StateId_t : int32_t
{
    kStateNil      = 0,
    kStateReady    = 1,
    kStateExit     = 2,
};

/**
 * @brief Enumeration for events.
 */
enum Event_t : uint64_t
{
    kEventNil               = 0,
    kEventNewKeysRequest    = 1,
    kEventDeleteKeysRequest = 2,
};

/**
 * @brief Provides 'Ready' state.
 */
struct StateReady : public stdsc::State
{
    static std::shared_ptr<State> create();
    StateReady(void);
    virtual void set(stdsc::StateContext& sc, uint64_t event) override;
    STDSC_STATE_DEFID(kStateReady);
};


} /* namespace fts_dec */

#endif /* FTS_DEC_SRV_STATE_HPP */
