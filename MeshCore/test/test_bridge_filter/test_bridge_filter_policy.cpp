#include <gtest/gtest.h>
#include <string.h>
#include <string>
#include "helpers/bridges/BridgeFilter.h"

// The native build uses the test mocks: SHA256 is deterministic but not cryptographic,
// and the mocked HMAC is always zero. So a group packet whose two MAC bytes are 0x00 0x00
// "verifies" under every key, and any other MAC fails under every key. That makes the
// channel-hash-byte matching the only thing deciding which blocked key applies, which is
// exactly what these tests exercise.

using mesh::BridgeFilter;
using mesh::Packet;
using Policy = BridgeFilter::BridgeFilterPolicy;

static const uint8_t PUBLIC_HASH = 0x11;

static Policy emptyPolicy() {
  Policy p;
  memset(&p, 0, sizeof(p));
  return p;
}

static Packet grpTxt(uint8_t channel_hash, bool mac_valid, uint8_t cipher_len = 16) {
  Packet pkt;
  memset(&pkt, 0, sizeof(pkt));
  pkt.header = (uint8_t)((PAYLOAD_TYPE_GRP_TXT << PH_TYPE_SHIFT) | ROUTE_TYPE_FLOOD);
  pkt.path_len = 0;
  pkt.payload[0] = channel_hash;
  pkt.payload[1] = mac_valid ? 0x00 : 0xFF;
  pkt.payload[2] = mac_valid ? 0x00 : 0xFF;
  for (int i = 0; i < cipher_len; i++) pkt.payload[3 + i] = (uint8_t)(0x41 + i);
  pkt.payload_len = (uint8_t)(3 + cipher_len);
  return pkt;
}

static Packet ofType(uint8_t type) {
  Packet pkt;
  memset(&pkt, 0, sizeof(pkt));
  pkt.header = (uint8_t)((type << PH_TYPE_SHIFT) | ROUTE_TYPE_FLOOD);
  pkt.payload_len = 8;
  return pkt;
}

// Block a hashtag through the normal path and return the channel hash byte its derived key produces.
static uint8_t blockTag(Policy& p, const char* name) {
  EXPECT_TRUE(BridgeFilter::addBlockedHTag(p, name));
  return BridgeFilter::channelHashByte(p.blockedPSKs[p.blockedHTagCount - 1]);
}

// Fill a slot directly with a chosen key (the mock SHA-256 derives every "#name" key from
// the same first byte, so distinct keys have to be set by hand), returning its hash byte.
static uint8_t setTag(Policy& p, const char* name, uint8_t key0) {
  uint8_t i = p.blockedHTagCount++;
  memset(p.blockedHTags[i], 0, BRIDGE_FILTER_BLOCKEDHTAGS_LEN);
  strcpy(p.blockedHTags[i], name);
  memset(p.blockedPSKs[i], 0, PUB_KEY_SIZE);
  p.blockedPSKs[i][0] = key0;
  for (int k = 1; k < 16; k++) p.blockedPSKs[i][k] = (uint8_t)(key0 * 7 + k);
  return BridgeFilter::channelHashByte(p.blockedPSKs[i]);
}

static void resetStats() {
  memset(BridgeFilter::bridgefilter_stats_htag_blocked, 0, sizeof(BridgeFilter::bridgefilter_stats_htag_blocked));
}

TEST(BridgeFilterPolicy, OnlyAdvertsAndGroupTextAreFiltered) {
  Policy p = emptyPolicy();
  Packet adv = ofType(PAYLOAD_TYPE_ADVERT);
  Packet txt = ofType(PAYLOAD_TYPE_TXT_MSG);
  EXPECT_TRUE(BridgeFilter::isPacketAllowed(p, &adv));
  EXPECT_TRUE(BridgeFilter::isPacketAllowed(p, &txt));
  BridgeFilter::blockAdverts(p);
  EXPECT_FALSE(BridgeFilter::isPacketAllowed(p, &adv));
  EXPECT_TRUE(BridgeFilter::isPacketAllowed(p, &txt));
}

TEST(BridgeFilterPolicy, PublicBlockedDropsOnlyVerifiedPublicPackets) {
  Policy p = emptyPolicy();
  BridgeFilter::blockPublic(p);
  Packet ok = grpTxt(PUBLIC_HASH, true);
  Packet bad_mac = grpTxt(PUBLIC_HASH, false);
  Packet other = grpTxt(PUBLIC_HASH + 1, true);
  EXPECT_FALSE(BridgeFilter::isPacketAllowed(p, &ok));
  EXPECT_TRUE(BridgeFilter::isPacketAllowed(p, &bad_mac));
  EXPECT_TRUE(BridgeFilter::isPacketAllowed(p, &other));
}

TEST(BridgeFilterPolicy, PublicNotBlockedPassesPublicPackets) {
  Policy p = emptyPolicy();
  Packet ok = grpTxt(PUBLIC_HASH, true);
  EXPECT_TRUE(BridgeFilter::isPacketAllowed(p, &ok));
}

TEST(BridgeFilterPolicy, HashtagIsMatchedByItsChannelHashByte) {
  resetStats();
  Policy p = emptyPolicy();
  uint8_t h = blockTag(p, "#test");
  Packet ok = grpTxt(h, true);
  Packet bad_mac = grpTxt(h, false);
  Packet other = grpTxt((uint8_t)(h ^ 0x5A), true);   // verifies under the mock, but another channel
  EXPECT_FALSE(BridgeFilter::isPacketAllowed(p, &ok));
  EXPECT_EQ(1u, BridgeFilter::bridgefilter_stats_htag_blocked[0]);
  EXPECT_TRUE(BridgeFilter::isPacketAllowed(p, &bad_mac));
  EXPECT_TRUE(BridgeFilter::isPacketAllowed(p, &other));
  EXPECT_EQ(1u, BridgeFilter::bridgefilter_stats_htag_blocked[0]);
}

TEST(BridgeFilterPolicy, HashtagSharingPublicsHashByteIsStillBlocked) {
  resetStats();
  Policy p = emptyPolicy();   // Public itself is NOT blocked
  uint8_t h = setTag(p, "#unlucky", PUBLIC_HASH);
  if (h != PUBLIC_HASH) GTEST_SKIP() << "mock hash does not map this key to 0x11";
  Packet ok = grpTxt(PUBLIC_HASH, true);
  EXPECT_FALSE(BridgeFilter::isPacketAllowed(p, &ok));
  EXPECT_EQ(1u, BridgeFilter::bridgefilter_stats_htag_blocked[0]);
}

TEST(BridgeFilterPolicy, FirstByteBlockDoesNotNeedAValidMac) {
  Policy p = emptyPolicy();
  BridgeFilter::addBlockedFirstByte(p, 0x42);
  Packet pkt = grpTxt(0x42, false);
  EXPECT_FALSE(BridgeFilter::isPacketAllowed(p, &pkt));
  Packet near = grpTxt(0x43, false);
  EXPECT_TRUE(BridgeFilter::isPacketAllowed(p, &near));
}

TEST(BridgeFilterPolicy, EachBlockedSlotCountsItsOwnHits) {
  resetStats();
  Policy p = emptyPolicy();
  uint8_t h1 = setTag(p, "#one", 0x31);
  uint8_t h2 = setTag(p, "#two", 0x32);
  ASSERT_NE(h1, h2);
  Packet second = grpTxt(h2, true);
  EXPECT_FALSE(BridgeFilter::isPacketAllowed(p, &second));
  EXPECT_EQ(0u, BridgeFilter::bridgefilter_stats_htag_blocked[0]);
  EXPECT_EQ(1u, BridgeFilter::bridgefilter_stats_htag_blocked[1]);
}

TEST(BridgeFilterPolicy, HashCacheFollowsAChangedSlot) {
  Policy p = emptyPolicy();
  uint8_t h1 = setTag(p, "#one", 0x31);
  Packet first = grpTxt(h1, true);
  EXPECT_FALSE(BridgeFilter::isPacketAllowed(p, &first));
  EXPECT_TRUE(BridgeFilter::deleteBlockedHTag(p, "#one"));
  uint8_t h2 = setTag(p, "#two", 0x32);   // reuses slot 0 with a different key
  ASSERT_NE(h1, h2);
  Packet second = grpTxt(h2, true);
  EXPECT_TRUE(BridgeFilter::isPacketAllowed(p, &first));
  EXPECT_FALSE(BridgeFilter::isPacketAllowed(p, &second));
}

TEST(BridgeFilterPolicy, EmptyGroupPayloadIsPassedWithoutReadingIt) {
  Policy p = emptyPolicy();
  BridgeFilter::addBlockedFirstByte(p, 0x42);
  Packet pkt = grpTxt(0x42, true);
  pkt.payload_len = 0;   // stale first byte must not be looked at
  EXPECT_TRUE(BridgeFilter::isPacketAllowed(p, &pkt));
}

// main() is in test_bridge_filter_format.cpp (one program per test directory)
