#ifndef FTS_PACKET_HPP
#define FTS_PACKET_HPP

#include <cstdint>

namespace fts_share
{

/**
 * @brief Enumeration for control code of packet.
 */
enum ControlCode_t : uint64_t
{
    /* Code for Request packet: 0x201-0x2FF */
    kControlCodeRequestNewKeys = 0x201,
    kControlCodeRequestDeleteKeys = 0x201,
    kControlCodeRequestQuery = 0x203,
    kControlCodeRequestResults = 0x204,

    /* Code for Data packet: 0x401-0x4FF */
    kControlCodeValueA = 0x401,
    kControlCodeValueB = 0x402,
    kControlCodeDataResult = 0x403,

    /* Code for Download packet: 0x801-0x8FF */
    kControlCodeDownloadResult = 0x801,
    kControlCodeDownloadQueryID = 0x802,
};

} /* namespace fts_share */

#endif /* FTS_PACKET_HPP */