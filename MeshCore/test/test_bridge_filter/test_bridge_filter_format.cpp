#include <gtest/gtest.h>
#include <string.h>
#include <string>
#include "helpers/bridges/BridgeFilter.h"

using mesh::BridgeFilter;
using Policy = BridgeFilter::BridgeFilterPolicy;

// The CLI reply buffers are 160 bytes (serial) and 161 bytes (remote admin, temp[166]+5).
static const size_t REPLY_LEN = 160;

// A reply buffer with guard bytes after it, so any write past out_len is detected.
struct GuardedReply {
  char raw[REPLY_LEN + 16];
  GuardedReply() { memset(raw, 0xA5, sizeof(raw)); }
  char* out() { return raw; }
  bool guardsIntact() const {
    for (size_t i = REPLY_LEN; i < sizeof(raw); i++) if ((uint8_t)raw[i] != 0xA5) return false;
    return true;
  }
};

static Policy emptyPolicy() {
  Policy p;
  memset(&p, 0, sizeof(p));
  return p;
}

// Fill the policy struct directly (no SHA-256) with a name and three distinct PSK bytes.
static void addTag(Policy& p, const char* name, uint8_t b0 = 0x01, uint8_t b1 = 0x02, uint8_t b2 = 0x03) {
  ASSERT_LT(p.blockedHTagCount, BRIDGE_FILTER_BLOCKEDHTAGS_MAX);
  ASSERT_LT(strlen(name), (size_t)BRIDGE_FILTER_BLOCKEDHTAGS_LEN);
  memset(p.blockedHTags[p.blockedHTagCount], 0, BRIDGE_FILTER_BLOCKEDHTAGS_LEN);
  strcpy(p.blockedHTags[p.blockedHTagCount], name);
  p.blockedPSKs[p.blockedHTagCount][0] = b0;
  p.blockedPSKs[p.blockedHTagCount][1] = b1;
  p.blockedPSKs[p.blockedHTagCount][2] = b2;
  p.blockedHTagCount++;
}

static std::string nameOfLen(size_t n, char fill) {
  std::string s = "#";
  while (s.size() < n) s += fill;
  return s;
}

// ── formatPolicy: exact text (format must not change for existing operators) ──

TEST(BridgeFilterFormat, PolicyNoneIsUnchanged) {
  Policy p = emptyPolicy();
  GuardedReply r;
  size_t n = BridgeFilter::formatPolicy(p, r.out(), REPLY_LEN);
  EXPECT_STREQ("> blocked: none\r\n? for help", r.out());
  EXPECT_EQ(strlen(r.out()), n);
  EXPECT_TRUE(r.guardsIntact());
}

TEST(BridgeFilterFormat, PolicyListsEverythingInOrder) {
  Policy p = emptyPolicy();
  BridgeFilter::blockAdverts(p);
  BridgeFilter::blockPublic(p);
  BridgeFilter::addBlockedFirstByte(p, 0x2a);
  BridgeFilter::addBlockedFirstByte(p, 0x0b);
  addTag(p, "#test", 0xde, 0xad, 0xbe);
  addTag(p, "#wardriving", 0x11, 0x22, 0x33);
  GuardedReply r;
  size_t n = BridgeFilter::formatPolicy(p, r.out(), REPLY_LEN);
  EXPECT_STREQ("> blocked: advert public 2a 0b #test (deadbe) #wardriving (112233)\r\n? for help", r.out());
  EXPECT_EQ(strlen(r.out()), n);
  EXPECT_TRUE(r.guardsIntact());
}

TEST(BridgeFilterFormat, PolicyOnlyAdvertOnlyPublic) {
  Policy p = emptyPolicy();
  BridgeFilter::blockAdverts(p);
  char out[REPLY_LEN];
  BridgeFilter::formatPolicy(p, out, sizeof(out));
  EXPECT_STREQ("> blocked: advert\r\n? for help", out);
  p = emptyPolicy();
  BridgeFilter::blockPublic(p);
  BridgeFilter::formatPolicy(p, out, sizeof(out));
  EXPECT_STREQ("> blocked: public\r\n? for help", out);
}

// ── formatPolicy: bounds ──

// "> blocked:" (10) + 3 x (1+30+9) + (1+7+9) + "\r\n? for help" (12) = 159: fits exactly.
static Policy policyOfTextLength159() {
  Policy p = emptyPolicy();
  addTag(p, nameOfLen(30, 'a').c_str());
  addTag(p, nameOfLen(30, 'b').c_str());
  addTag(p, nameOfLen(30, 'c').c_str());
  addTag(p, nameOfLen(7, 'd').c_str());
  return p;
}

TEST(BridgeFilterFormat, PolicyExactlyFittingTextIsNotCut) {
  Policy p = policyOfTextLength159();
  GuardedReply r;
  size_t n = BridgeFilter::formatPolicy(p, r.out(), REPLY_LEN);
  EXPECT_EQ(159u, n);
  EXPECT_EQ(159u, strlen(r.out()));
  EXPECT_EQ(0, r.out()[159]);
  EXPECT_TRUE(r.guardsIntact());
  std::string s(r.out());
  EXPECT_EQ(s.substr(s.size() - 12), "\r\n? for help");   // no "..." marker
}

TEST(BridgeFilterFormat, PolicyOneByteTooLongIsCutAndMarked) {
  Policy p = emptyPolicy();
  addTag(p, nameOfLen(30, 'a').c_str());
  addTag(p, nameOfLen(30, 'b').c_str());
  addTag(p, nameOfLen(30, 'c').c_str());
  addTag(p, nameOfLen(8, 'd').c_str());   // 160 chars: one more than fits
  GuardedReply r;
  size_t n = BridgeFilter::formatPolicy(p, r.out(), REPLY_LEN);
  EXPECT_EQ(160u, n);
  EXPECT_EQ(159u, strlen(r.out()));
  std::string s(r.out());
  EXPECT_EQ(s.substr(s.size() - 3), "...");
  EXPECT_TRUE(r.guardsIntact());
}

TEST(BridgeFilterFormat, PolicyWorstCaseStaysInBufferAndReportsFullLength) {
  Policy p = emptyPolicy();
  BridgeFilter::blockAdverts(p);
  BridgeFilter::blockPublic(p);
  for (int i = 0; i < BRIDGE_FILTER_BLOCKEDFIRSTBYTES_MAX; i++) BridgeFilter::addBlockedFirstByte(p, (uint8_t)i);
  for (int i = 0; i < BRIDGE_FILTER_BLOCKEDHTAGS_MAX; i++) addTag(p, nameOfLen(30, 'a' + i).c_str());
  GuardedReply r;
  size_t n = BridgeFilter::formatPolicy(p, r.out(), REPLY_LEN);
  // 10 + 7 + 7 + 8*3 + 16*40 + 12 = 700
  EXPECT_EQ(700u, n);
  EXPECT_EQ(159u, strlen(r.out()));
  EXPECT_TRUE(r.guardsIntact());
  // The visible part is a prefix of the full text (plus the marker).
  char full[1024];
  BridgeFilter::formatPolicy(p, full, sizeof(full));
  EXPECT_EQ(700u, strlen(full));
  EXPECT_EQ(0, memcmp(full, r.out(), 156));
  EXPECT_STREQ("...", r.out() + 156);
}

TEST(BridgeFilterFormat, PolicyTinyBuffersNeverOverflow) {
  Policy p = emptyPolicy();
  addTag(p, "#test");
  for (size_t cap = 0; cap <= 8; cap++) {
    char raw[16];
    memset(raw, 0xA5, sizeof(raw));
    size_t n = BridgeFilter::formatPolicy(p, raw, cap);
    EXPECT_GT(n, cap);
    for (size_t i = cap; i < sizeof(raw); i++) EXPECT_EQ(0xA5, (uint8_t)raw[i]) << "cap=" << cap << " i=" << i;
    if (cap > 0) EXPECT_LT(strlen(raw), cap);
  }
}

TEST(BridgeFilterFormat, PolicyUnterminatedNameIsReadOnlyToItsSlot) {
  // A corrupt prefs file can leave a name without its NUL; the reply must not read past the slot.
  Policy p = emptyPolicy();
  memset(p.blockedHTags[0], 'x', BRIDGE_FILTER_BLOCKEDHTAGS_LEN);   // no terminator
  memset(p.blockedHTags[1], 'y', BRIDGE_FILTER_BLOCKEDHTAGS_LEN);   // the slot after it
  p.blockedHTagCount = 1;
  char out[256];
  BridgeFilter::formatPolicy(p, out, sizeof(out));
  std::string s(out);
  EXPECT_EQ(std::string::npos, s.find('y'));
  EXPECT_NE(std::string::npos, s.find(std::string(BRIDGE_FILTER_BLOCKEDHTAGS_LEN - 1, 'x') + " ("));
}

// ── formatStats ──

TEST(BridgeFilterFormat, StatsExactText) {
  Policy p = emptyPolicy();
  addTag(p, "#test");
  addTag(p, "#bot");
  BridgeFilter::bridgefilter_stats_tx_sent = 3322;
  BridgeFilter::bridgefilter_stats_tx_blocked = 929;
  BridgeFilter::bridgefilter_stats_rx_received = 886;
  BridgeFilter::bridgefilter_stats_rx_blocked = 269;
  BridgeFilter::bridgefilter_stats_htag_blocked[0] = 1142;
  BridgeFilter::bridgefilter_stats_htag_blocked[1] = 7;
  GuardedReply r;
  size_t n = BridgeFilter::formatStats(p, r.out(), REPLY_LEN);
  EXPECT_STREQ("> TX: 3322 sent, 929 blocked. RX: 886 received, 269 blocked. #test: 1142 blocked. #bot: 7 blocked.", r.out());
  EXPECT_EQ(strlen(r.out()), n);
  EXPECT_TRUE(r.guardsIntact());
}

TEST(BridgeFilterFormat, StatsWorstCaseStaysInBuffer) {
  Policy p = emptyPolicy();
  for (int i = 0; i < BRIDGE_FILTER_BLOCKEDHTAGS_MAX; i++) {
    addTag(p, nameOfLen(30, 'a' + i).c_str());
    BridgeFilter::bridgefilter_stats_htag_blocked[i] = 4000000000u;
  }
  BridgeFilter::bridgefilter_stats_tx_sent = 4000000000u;
  GuardedReply r;
  size_t n = BridgeFilter::formatStats(p, r.out(), REPLY_LEN);
  EXPECT_GT(n, REPLY_LEN);
  EXPECT_EQ(159u, strlen(r.out()));
  EXPECT_STREQ("...", r.out() + 156);
  EXPECT_TRUE(r.guardsIntact());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
