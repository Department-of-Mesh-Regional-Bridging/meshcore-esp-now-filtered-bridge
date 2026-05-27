#pragma once

#include "Packet.h"
#include "Utils.h"
#include <MeshCore.h>

#define BRIDGE_FILTER_BLOCKEDCHANNELS_MAX 8

namespace mesh {

  class PacketFilter {
  public:
    /**
     * \brief  examine a packet and decide to allow / drop in a bridge
     */

    struct BridgeFilterPolicy {
      uint8_t policy;
      uint8_t blockedChannels[BRIDGE_FILTER_BLOCKEDCHANNELS_MAX];
      uint8_t blockedChannelCount = 0;
    };

    // Statistics
    static uint32_t bridgefilter_stats_tx_sent;
    static uint32_t bridgefilter_stats_tx_blocked;
    static uint32_t bridgefilter_stats_rx_received;
    static uint32_t bridgefilter_stats_rx_blocked;

    // Policy
    static bool isPacketAllowed(const BridgeFilterPolicy& bridge_filter_policy, mesh::Packet *pkt);
    static bool isPolicyEnabled(const BridgeFilterPolicy &bridge_filter_policy) {
      return bridge_filter_policy.policy > 0 || bridge_filter_policy.blockedChannelCount > 0;
    }

    static void clearPolicy(BridgeFilterPolicy& bridge_filter_policy) {
      bridge_filter_policy.policy = 0;
      bridge_filter_policy.blockedChannelCount = 0;
    }

    // Adverts
    static bool isAdvertsBlocked(const BridgeFilterPolicy& bridge_filter_policy) { return bridge_filter_policy.policy & 0b00000001; }
    static void blockAdverts(BridgeFilterPolicy& bridge_filter_policy) { bridge_filter_policy.policy |= 0b00000001; }
    
    // Public channel
    static bool isPublicBlocked(const BridgeFilterPolicy& bridge_filter_policy) { return bridge_filter_policy.policy & 0b00000010; }
    static void blockPublic(BridgeFilterPolicy& bridge_filter_policy) { bridge_filter_policy.policy |= 0b00000010; }

    // Blocked channels by first 1-byte channel hash
    static bool isChannelBlocked(const BridgeFilterPolicy& bridge_filter_policy, uint8_t channel_hash_1byte);
    static bool addBlockedChannel(BridgeFilterPolicy& bridge_filter_policy, uint8_t channel_hash_1byte);
    static bool deleteBlockedChannel(BridgeFilterPolicy &bridge_filter_policy, uint8_t channel_hash_1byte);
  }; // PacketFilter
} // namespace mesh
