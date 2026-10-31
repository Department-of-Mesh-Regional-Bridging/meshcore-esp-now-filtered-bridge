#ifdef ARDUINO
#include <Arduino.h>
#endif

#include "BridgeFilter.h"
#include <SHA256.h>
#include <stdarg.h>
#include <stdio.h>

namespace mesh {
  // Statistics
  uint32_t mesh::BridgeFilter::bridgefilter_stats_tx_sent = 0;
  uint32_t mesh::BridgeFilter::bridgefilter_stats_tx_blocked = 0;
  uint32_t mesh::BridgeFilter::bridgefilter_stats_rx_received = 0;
  uint32_t mesh::BridgeFilter::bridgefilter_stats_rx_blocked = 0;
  uint32_t mesh::BridgeFilter::bridgefilter_stats_htag_blocked[BRIDGE_FILTER_BLOCKEDHTAGS_MAX] = { 0};

  // https://emn178.github.io/online-tools/sha256.html
  // Private key of Public channel: 8b3387e9c5cdea6ac9e5edbaa115cd72
  // SHA256=>1155f1870f91778646972b7bde747d989cf9f95416b50829666036563365424d
  // First byte is 0x11

  // Payload : 11 5D4A8A5FA5912A52244139E36F0BB43612967906F13DE5692C1030885AECF285AD9B
  bool BridgeFilter::isPacketAllowed(const BridgeFilterPolicy& policy, mesh::Packet *pkt) {
    BRIDGEFILTER_DEBUG_PRINTLN("Payload type: %s, first byte: %02x", pkt->getPayloadTypeText(), pkt->payload[0]);

    // Drop adverts
    if (isAdvertsBlocked(policy) && pkt->getPayloadType() == PAYLOAD_TYPE_ADVERT) {
      BRIDGEFILTER_DEBUG_PRINTLN("Advert message");
      return false; // Drop
    }

    // Drop message based on first-byte channel hash
    if (pkt->getPayloadType() == PAYLOAD_TYPE_GRP_TXT) {
      if (isBlockedFirstByte(policy, pkt->payload[0])) {
        BRIDGEFILTER_DEBUG_PRINTLN("First-byte message");
        return false; // Drop
      }
    }

    // Drop message from channels
    if (pkt->getPayloadType() == PAYLOAD_TYPE_GRP_TXT) {
      if (pkt->payload[0] == 0x11) { // "Public" channel?
        // Known public key of "Public" channel
        uint8_t shared_secret[PUB_KEY_SIZE] = { 0x8b, 0x33, 0x87, 0xe9, 0xc5, 0xcd, 0xea, 0x6a,
                                                0xc9, 0xe5, 0xed, 0xba, 0xa1, 0x15, 0xcd, 0x72 };
        uint8_t dest[MAX_PACKET_PAYLOAD];
        int len = mesh::Utils::MACThenDecrypt(shared_secret, dest, &pkt->payload[1], pkt->payload_len - 1);
        if (len > 0) { // Able to decode
          // Public message but not blocked
          if (!isPublicBlocked(policy)) {
            return true;
          }

          // Decoded message
          char *message = (char *)&dest[5];
          BRIDGEFILTER_DEBUG_PRINTLN("Public message: %s", message);
          return false; // Block
        } else {        // Failed to decode
          BRIDGEFILTER_DEBUG_PRINTLN("Not Public message");
        }
      } else { // Check for blocked Hashtags
        for (int i = 0; i < policy.blockedHTagCount; i++) {
          // Get Hashtag name
          const char* hashtag = policy.blockedHTags[i];

          uint8_t dest[MAX_PACKET_PAYLOAD];
          int len = mesh::Utils::MACThenDecrypt(policy.blockedPSKs[i], dest, &pkt->payload[1], pkt->payload_len - 1);
          if (len > 0) { // Able to decode
            // Decoded message
            char *message = (char *)&dest[5];
            BRIDGEFILTER_DEBUG_PRINTLN("Hashtag %s message: %s", hashtag, message);

            // Statistics
            bridgefilter_stats_htag_blocked[i]++;
            return false; // Block
          } else {
            BRIDGEFILTER_DEBUG_PRINTLN("Not hashtag %s message", hashtag);
          }
        }
      } // End of check for blocked Hashtags
    } // End of drop message from channels

    return true; // Allow if reach here
  } // isPacketAllowed

  // First byte
  bool BridgeFilter::isBlockedFirstByte(const BridgeFilterPolicy& policy, uint8_t channel_hash_1byte) {
    for (uint8_t i = 0; i < policy.blockedFirstBytesCount; i++) {
      if (policy.blockedFirstBytes[i] == channel_hash_1byte) {
        return true;
      }
    }

    return false;
  }

  bool BridgeFilter::addBlockedFirstByte(BridgeFilterPolicy& policy, uint8_t channel_hash_1byte) {
    // avoid duplicates
    if (isBlockedFirstByte(policy, channel_hash_1byte)) {
      return true;
    }

    // avoid overflow
    if (policy.blockedFirstBytesCount >= BRIDGE_FILTER_BLOCKEDFIRSTBYTES_MAX) {
      return false;
    }

    policy.blockedFirstBytes[policy.blockedFirstBytesCount++] = channel_hash_1byte;
    return true;
  }

  bool BridgeFilter::deleteBlockedFirstByte(BridgeFilterPolicy &policy, uint8_t channel_hash_1byte) {
    if (!isBlockedFirstByte(policy, channel_hash_1byte)) {
      return false;
    }

    for (uint8_t i = 0; i < policy.blockedFirstBytesCount; i++) {
      // Found target channel in blocked list
      if (policy.blockedFirstBytes[i] == channel_hash_1byte) {
        // Remove by swapping with the last element (O(1) removal, no shifting needed)
        policy.blockedFirstBytes[i] = policy.blockedFirstBytes[policy.blockedFirstBytesCount - 1];

        // Reduce active count (logical deletion)
        policy.blockedFirstBytesCount--;

        // Optional: clear last slot for debugging / hygiene (not required for logic)
        policy.blockedFirstBytes[policy.blockedFirstBytesCount] = 0;
        return true; // successfully removed
      }
    }

    return false; // Not found
  }

  // Hashtag
  bool BridgeFilter::isBlockedHTag(const BridgeFilterPolicy &policy, const char* htag) {
    for (uint8_t i = 0; i < policy.blockedHTagCount; i++) {
      if (memcmp(policy.blockedHTags[i], htag, BRIDGE_FILTER_BLOCKEDHTAGS_LEN) == 0) {
        return true;
      }
    }
    return false;
  }

  bool BridgeFilter::addBlockedHTag(BridgeFilterPolicy &policy, const char* htag) {
    // avoid duplicates
    for (uint8_t i = 0; i < policy.blockedHTagCount; i++) {
      if (strcmp(policy.blockedHTags[i], htag) == 0) {
        return true;
      }
    }

    // avoid overflow
    if (policy.blockedHTagCount >= BRIDGE_FILTER_BLOCKEDHTAGS_MAX) {
      return false;
    }

    // must fit in destination buffer including '\0'
    if (strlen(htag) >= BRIDGE_FILTER_BLOCKEDHTAGS_LEN) {
      return false;
    }

    // Add hashtag
    memset(policy.blockedHTags[policy.blockedHTagCount], 0, BRIDGE_FILTER_BLOCKEDHTAGS_LEN);
    strcpy(policy.blockedHTags[policy.blockedHTagCount], htag);

    // Get PSK of hashtag
    uint8_t shared_secret[PUB_KEY_SIZE] = { 0 };
    SHA256 sha;
    sha.update(htag, strlen(htag));
    sha.finalize(shared_secret, 16);

    // Add PSK
    memcpy(policy.blockedPSKs[policy.blockedHTagCount], shared_secret, PUB_KEY_SIZE);
    policy.blockedHTagCount++;
    return true;
  }

  bool BridgeFilter::deleteBlockedHTag(BridgeFilterPolicy &policy, const char *htag) {
    for (uint8_t i = 0; i < policy.blockedHTagCount; i++) {
      if (strcmp(policy.blockedHTags[i], htag) == 0) {
        for (uint8_t j = i; j + 1 < policy.blockedHTagCount; j++) {
          memcpy(policy.blockedHTags[j], policy.blockedHTags[j + 1], BRIDGE_FILTER_BLOCKEDHTAGS_LEN);
          memcpy(policy.blockedPSKs[j], policy.blockedPSKs[j + 1], PUB_KEY_SIZE);
          mesh::BridgeFilter::bridgefilter_stats_htag_blocked[j] = mesh::BridgeFilter::bridgefilter_stats_htag_blocked[j + 1];
        }

        // clear last slot (important for flash consistency)
        memset(policy.blockedHTags[policy.blockedHTagCount - 1], 0, BRIDGE_FILTER_BLOCKEDHTAGS_LEN);
        memset(policy.blockedPSKs[policy.blockedHTagCount - 1], 0, PUB_KEY_SIZE);
        mesh::BridgeFilter::bridgefilter_stats_htag_blocked[policy.blockedHTagCount - 1] = 0;

        // Reduce the count
        policy.blockedHTagCount--;
        return true;
      }
    }

    return false;
  }

  // CLI text
  namespace {
    // Appends formatted text to a fixed buffer without ever writing past it, while
    // counting the length the complete text would need (like snprintf).
    struct BoundedWriter {
      char* out;
      size_t cap;
      size_t len = 0;   // length the full text needs, may exceed cap
      BoundedWriter(char* o, size_t c) : out(o), cap(c) { if (cap > 0) out[0] = 0; }
      void append(const char* fmt, ...) {
        va_list ap;
        va_start(ap, fmt);
        size_t room = (len < cap) ? cap - len : 0;
        int n = vsnprintf(room > 0 ? out + len : NULL, room, fmt, ap);
        va_end(ap);
        if (n > 0) len += (size_t)n;
      }
      // When the text did not fit, end the visible part with "..." so the reader knows.
      size_t finish() {
        if (len >= cap && cap >= 4) memcpy(out + cap - 4, "...", 4);
        return len;
      }
    };
  }

  size_t BridgeFilter::formatPolicy(const BridgeFilterPolicy& policy, char* out, size_t out_len) {
    BoundedWriter w(out, out_len);
    if (!isPolicyEnabled(policy)) {
      w.append("> blocked: none");
    } else {
      w.append("> blocked:");
      if (isAdvertsBlocked(policy)) w.append(" advert");
      if (isPublicBlocked(policy)) w.append(" public");
      for (uint8_t i = 0; i < policy.blockedFirstBytesCount; i++) {
        w.append(" %02x", policy.blockedFirstBytes[i]);
      }
      for (uint8_t i = 0; i < policy.blockedHTagCount; i++) {
        w.append(" %.*s (%02x%02x%02x)", BRIDGE_FILTER_BLOCKEDHTAGS_LEN - 1, policy.blockedHTags[i],
                 policy.blockedPSKs[i][0], policy.blockedPSKs[i][1], policy.blockedPSKs[i][2]);
      }
    }
    w.append("\r\n? for help");
    return w.finish();
  }

  size_t BridgeFilter::formatStats(const BridgeFilterPolicy& policy, char* out, size_t out_len) {
    BoundedWriter w(out, out_len);
    w.append("> TX: %lu sent, %lu blocked. RX: %lu received, %lu blocked.",
             (unsigned long)bridgefilter_stats_tx_sent, (unsigned long)bridgefilter_stats_tx_blocked,
             (unsigned long)bridgefilter_stats_rx_received, (unsigned long)bridgefilter_stats_rx_blocked);
    for (uint8_t i = 0; i < policy.blockedHTagCount; i++) {
      w.append(" %.*s: %lu blocked.", BRIDGE_FILTER_BLOCKEDHTAGS_LEN - 1, policy.blockedHTags[i],
               (unsigned long)bridgefilter_stats_htag_blocked[i]);
    }
    return w.finish();
  }
} // namespace mesh
