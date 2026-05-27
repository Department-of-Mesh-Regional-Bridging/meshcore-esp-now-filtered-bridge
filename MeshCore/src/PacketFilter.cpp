#include "PacketFilter.h"

#ifdef ARDUINO
#include <Arduino.h>
#endif

namespace mesh {
  // Statistics
  uint32_t mesh::PacketFilter::bridgefilter_stats_tx_sent = 0;
  uint32_t mesh::PacketFilter::bridgefilter_stats_tx_blocked = 0;
  uint32_t mesh::PacketFilter::bridgefilter_stats_rx_received = 0;
  uint32_t mesh::PacketFilter::bridgefilter_stats_rx_blocked = 0;

  // https://emn178.github.io/online-tools/sha256.html
  // Private key of Public channel: 8b3387e9c5cdea6ac9e5edbaa115cd72
  // SHA256=>1155f1870f91778646972b7bde747d989cf9f95416b50829666036563365424d
  // First byte is 0x11

  // Payload : 11 5D4A8A5FA5912A52244139E36F0BB43612967906F13DE5692C1030885AECF285AD9B
  bool PacketFilter::isPacketAllowed(const BridgeFilterPolicy& bridge_filter_policy, mesh::Packet *pkt) {
    PACKETFILTER_DEBUG_PRINTLN("Payload type: %s, first byte: %02x", pkt->getPayloadTypeText(), pkt->payload[0]);

    // Drop adverts
    if (isAdvertsBlocked(bridge_filter_policy) && pkt->getPayloadType() == PAYLOAD_TYPE_ADVERT) {
      PACKETFILTER_DEBUG_PRINTLN("Dropped adverts");
      return false; // Drop
    }

    // Drop message based on 1-byte channel hash
    if (pkt->getPayloadType() == PAYLOAD_TYPE_GRP_TXT) {
      if (isChannelBlocked(bridge_filter_policy, pkt->payload[0])) {
        PACKETFILTER_DEBUG_PRINTLN("Dropped message due to first byte");
        return false; // Drop
      }
    }

    // Drop message from Public channel
    if (pkt->getPayloadType() == PAYLOAD_TYPE_GRP_TXT) {
      if (pkt->payload[0] == 0x11) {
        uint8_t shared_secret[PUB_KEY_SIZE] = { 0x8b, 0x33, 0x87, 0xe9, 0xc5, 0xcd, 0xea, 0x6a,
                                                0xc9, 0xe5, 0xed, 0xba, 0xa1, 0x15, 0xcd, 0x72 };
        uint8_t dest[MAX_PACKET_PAYLOAD];
        int len = mesh::Utils::MACThenDecrypt(shared_secret, dest, &pkt->payload[1], pkt->payload_len - 1);
        if (len > 0) { // Able to decode
          // Public message but not blocked
          if (!isPublicBlocked(bridge_filter_policy)) {
            return true;
          }

          // Timestamp
          uint32_t timestamp = dest[0] | (dest[1] << 8) | (dest[2] << 16) | (dest[3] << 24);
          time_t t = timestamp;
          struct tm *tm_info = localtime(&t);
          PACKETFILTER_DEBUG_PRINTLN("Date time: %02d/%02d/%04d %02d:%02d:%02d", tm_info->tm_mday,
                                     tm_info->tm_mon + 1, tm_info->tm_year + 1900, tm_info->tm_hour,
                                     tm_info->tm_min, tm_info->tm_sec);

          // Decoded message
          char *message = (char *)&dest[5];
          PACKETFILTER_DEBUG_PRINTLN("Decoded message: %s", message);
          return false; // Able to decode, it is a message in Public channel
        } else {        // Failed to decode
          PACKETFILTER_DEBUG_PRINTLN("Failed to decode. Not Public message");
        }
      }

      return true; // Allow if reach here
    }
  } // isPacketAllowed

  bool PacketFilter::isChannelBlocked(const BridgeFilterPolicy& bridge_filter_policy, uint8_t channel_hash_1byte) {
    for (uint8_t i = 0; i < bridge_filter_policy.blockedChannelCount; i++) {
      if (bridge_filter_policy.blockedChannels[i] == channel_hash_1byte) {
        return true;
      }
    }

    return false;
  }

  bool PacketFilter::addBlockedChannel(BridgeFilterPolicy& bridge_filter_policy, uint8_t channel_hash_1byte) {
    // avoid duplicates
    if (isChannelBlocked(bridge_filter_policy, channel_hash_1byte)) {
      return true;
    }

    // avoid overflow
    if (bridge_filter_policy.blockedChannelCount >= BRIDGE_FILTER_BLOCKEDCHANNELS_MAX) {
      return false;
    }

    bridge_filter_policy.blockedChannels[bridge_filter_policy.blockedChannelCount++] = channel_hash_1byte;

    return true;
  }

  bool PacketFilter::deleteBlockedChannel(BridgeFilterPolicy &bridge_filter_policy, uint8_t channel_hash_1byte) {
    if (!isChannelBlocked(bridge_filter_policy, channel_hash_1byte)) {
      return false;
    }

    for (uint8_t i = 0; i < bridge_filter_policy.blockedChannelCount; i++) {
      if (bridge_filter_policy.blockedChannels[i] == channel_hash_1byte) {
        // Found target channel in blocked list

        // Remove by swapping with the last element (O(1) removal, no shifting needed)
        bridge_filter_policy.blockedChannels[i] =
            bridge_filter_policy.blockedChannels[bridge_filter_policy.blockedChannelCount - 1];

        // Reduce active count (logical deletion)
        bridge_filter_policy.blockedChannelCount--;

        // Optional: clear last slot for debugging / hygiene (not required for logic)
        bridge_filter_policy.blockedChannels[bridge_filter_policy.blockedChannelCount] = 0;
        return true; // successfully removed
      }
    }

    return false; // Not found
  }
} // namespace mesh