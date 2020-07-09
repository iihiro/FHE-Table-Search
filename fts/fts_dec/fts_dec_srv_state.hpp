#ifndef FTS_DEC_SRV_STATE_HPP
#define FTS_DEC_SRV_STATE_HPP

#include <memory>
#include <cstdbool>
#include <stdsc/stdsc_state.hpp>

namespace fts_dec_server
{

/**
 * @brief Enumeration for state.
 */
enum StateId_t : int32_t
{
    kStateNil      = 0,
    kStateInit     = 1,
    kStateReady    = 2,
    kStateComputed = 3,
};

/**
 * @brief Enumeration for events.
 */
enum Event_t : uint64_t
{
    kEventReceivedNewKeysRequest = 3,
    kEventReceivedDeleteKeysRequest = 4,
    kEventReceivedResultRequest  = 5,
};

/**
 * @brief Provides 'Connected' state.
 */
struct StateInit : public stdsc::State
{
    static std::shared_ptr<stdsc::State> create();
    StateInit(void);
    virtual void set(stdsc::StateContext& sc, uint64_t event) override;
    STDSC_STATE_DEFID(kStateInit);

private:
    bool is_received_valueA_;
    bool is_received_valueB_;
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

/**
 * @brief Provides 'Computed' state.
 */
struct StateComputed : public stdsc::State
{
    static std::shared_ptr<State> create();
    StateComputed(void);
    virtual void set(stdsc::StateContext& sc, uint64_t event) override;
    STDSC_STATE_DEFID(kStateComputed);
};


} /* namespace fts_dec_server */

#endif /* FTS_DEC_SRV_STATE_HPP */