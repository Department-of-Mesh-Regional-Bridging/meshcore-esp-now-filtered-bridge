#pragma once

#include "Packet.h"
#include "Utils.h"
#include <MeshCore.h>

#define BRIDGE_FILTER_BLOCKEDFIRSTBYTES_MAX 8
#define BRIDGE_FILTER_BLOCKEDHTAGS_MAX 16
#define BRIDGE_FILTER_BLOCKEDHTAGS_LEN 31
#define BRIDGE_FILTER_BLOCKEDPSKS_MAX 16

namespace mesh {

  class BridgeFilter {
  public:
    /**
     * \brief  examine a packet and decide to allow / drop in a bridge
     */

    struct BridgeFilterPolicy {
      uint8_t policy;
      
      // 1-byte blocking
      uint8_t blockedFirstBytes[BRIDGE_FILTER_BLOCKEDFIRSTBYTES_MAX];
      uint8_t blockedFirstBytesCount = 0;

      // Hashtag blocking
      char blockedHTags[BRIDGE_FILTER_BLOCKEDHTAGS_MAX][BRIDGE_FILTER_BLOCKEDHTAGS_LEN];
      uint8_t blockedPSKs[BRIDGE_FILTER_BLOCKEDPSKS_MAX][PUB_KEY_SIZE]; // Keep PSK of hashtags
      uint8_t blockedHTagCount = 0;
    };

    // Statistics
    static uint32_t bridgefilter_stats_tx_sent;
    static uint32_t bridgefilter_stats_tx_blocked;
    static uint32_t bridgefilter_stats_rx_received;
    static uint32_t bridgefilter_stats_rx_blocked;
    static uint32_t bridgefilter_stats_htag_blocked[BRIDGE_FILTER_BLOCKEDHTAGS_MAX];

    // Policy
    static bool isPacketAllowed(const BridgeFilterPolicy& policy, mesh::Packet *pkt);
    static bool isPolicyEnabled(const BridgeFilterPolicy &policy) {
      return policy.policy > 0 || policy.blockedFirstBytesCount > 0 || policy.blockedHTagCount > 0;
    }

    static void clearPolicy(BridgeFilterPolicy& policy) {
      policy.policy = 0;
      policy.blockedFirstBytesCount = 0;
      policy.blockedHTagCount = 0;
    }

    // Adverts
    static bool isAdvertsBlocked(const BridgeFilterPolicy& policy) { return policy.policy & 0b00000001; }
    static void blockAdverts(BridgeFilterPolicy& policy) { policy.policy |= 0b00000001; }
    static void unblockAdverts(BridgeFilterPolicy& policy) { policy.policy &= ~0b00000001; }
    
    // Public channel
    static bool isPublicBlocked(const BridgeFilterPolicy& policy) { return policy.policy & 0b00000010; }
    static void blockPublic(BridgeFilterPolicy& policy) { policy.policy |= 0b00000010; }
    static void unblockPublic(BridgeFilterPolicy& policy) { policy.policy &= ~0b00000010; }

    // Blocked channels by first 1-byte channel hash
    static bool isBlockedFirstByte(const BridgeFilterPolicy& policy, uint8_t channel_hash_1byte);
    static bool addBlockedFirstByte(BridgeFilterPolicy& policy, uint8_t channel_hash_1byte);
    static bool deleteBlockedFirstByte(BridgeFilterPolicy &policy, uint8_t channel_hash_1byte);

    // Blocked channels by PSKs
    static bool isBlockedHTag(const BridgeFilterPolicy &policy, const char* htag);
    static bool addBlockedHTag(BridgeFilterPolicy& policy, const char* htag);
    static bool deleteBlockedHTag(BridgeFilterPolicy &policy, const char* htag);

  }; // BridgeFilter
} // namespace mesh
