#include <cxxtest/TestSuite.h>

#include "common/list.h"

#include "sci/engine/tsn_online_executive.h"
#include "sci/engine/tsn_transport.h"

namespace {

// Envelopes as hex text, length prefix included: crates/int14h/tests/golden/envelopes.txt
byte hexDigit(char digit) {
	return digit <= '9' ? digit - '0' : digit - 'a' + 10;
}

Common::Array<byte> fromHex(const char *text) {
	Common::Array<byte> bytes;
	for (const char *digit = text; *digit; ++digit) {
		if (*digit == ' ')
			continue;
		bytes.push_back(hexDigit(digit[0]) << 4 | hexDigit(digit[1]));
		++digit;
	}
	return bytes;
}

const char *const kHello = "10 00 00 00 01 00 00 49 31 34 48 01 07 73 63 75 6d 6d 76 6d";
const char *const kWelcome = "0f 00 00 00 02 00 00 01 0a 69 6e 6e 6b 65 65 70 65 72 64";
const char *const kConnect = "0a 00 00 00 10 01 00 03 05 41 54 44 54 30";
const char *const kConnected = "05 00 00 00 11 01 00 03 00";
const char *const kSend = "09 00 00 00 10 02 00 04 03 00 22 04 10";
const char *const kQueued = "05 00 00 00 11 02 00 04 01";
const char *const kPoll = "04 00 00 00 10 03 00 08";
const char *const kPollOk = "05 00 00 00 11 03 00 08 00";
const char *const kReceive = "04 00 00 00 10 04 00 05";
const char *const kReceived = "09 00 00 00 11 04 00 05 01 02 00 07 41";
const char *const kReceiveEmpty = "05 00 00 00 11 05 00 05 00";

// A host that answers with prepared stream chunks and records what it was sent
class LoopbackTransport : public Sci::TsnTransport {
public:
	LoopbackTransport() : reachable(true), opens(0), closes(0) {}

	bool open() override { ++opens; return reachable; }
	void close() override { ++closes; }
	bool send(const Common::Array<byte> &bytes) override { sent.push_back(bytes); return true; }
	bool receive(Common::Array<byte> &bytes) override {
		if (replies.empty())
			return false;
		bytes.push_back(replies.front());
		replies.pop_front();
		return true;
	}

	void reply(const char *hex) { replies.push_back(fromHex(hex)); }

	bool reachable;
	int opens;
	int closes;
	Common::Array<byte> sent;
	Common::List<Common::Array<byte> > replies;
};

} // End of anonymous namespace

class TsnOnlineExecutiveTestSuite : public CxxTest::TestSuite {
	Common::Array<byte> stream(const char *const *envelopes, uint count) {
		Common::Array<byte> bytes;
		for (uint i = 0; i < count; ++i)
			bytes.push_back(fromHex(envelopes[i]));
		return bytes;
	}

public:
	void test_answers_as_an_idle_line_until_dialed() {
		LoopbackTransport *host = new LoopbackTransport();
		Sci::OnlineTsnExecutive executive(host);
		TS_ASSERT_EQUALS(executive.getStatus().toWord(), 3);
		TS_ASSERT_EQUALS(executive.poll(), Sci::kTsnLinkOk);
		TS_ASSERT(!executive.send(fromHex("22")));
		TS_ASSERT_EQUALS(executive.getLineRate(), 1200);
		TS_ASSERT_EQUALS(host->opens, 0);
		TS_ASSERT(host->sent.empty());
	}

	void test_golden_call_sequence() {
		LoopbackTransport *host = new LoopbackTransport();
		const char *const replies[] = { kWelcome, kConnected, kQueued, kPollOk, kReceived, kReceiveEmpty };
		for (uint i = 0; i < ARRAYSIZE(replies); ++i)
			host->reply(replies[i]);
		Sci::OnlineTsnExecutive executive(host);

		TS_ASSERT_EQUALS(executive.connect("ATDT0"), Sci::kTsnConnected);
		TS_ASSERT(executive.send(fromHex("22 04 10")));
		TS_ASSERT_EQUALS(executive.poll(), Sci::kTsnLinkOk);
		Common::Array<byte> body;
		TS_ASSERT(executive.receive(body));
		TS_ASSERT(body == fromHex("07 41"));
		TS_ASSERT(!executive.receive(body));

		const char *const calls[] = { kHello, kConnect, kSend, kPoll, kReceive, "04 00 00 00 10 05 00 05" };
		TS_ASSERT(host->sent == stream(calls, ARRAYSIZE(calls)));
	}

	void test_status_and_local_exports() {
		LoopbackTransport *host = new LoopbackTransport();
		host->reply(kWelcome);
		host->reply(kConnected);
		host->reply("06 00 00 00 11 02 00 00 03 80");
		Sci::OnlineTsnExecutive executive(host);
		TS_ASSERT_EQUALS(executive.connect("ATDT0"), Sci::kTsnConnected);
		TS_ASSERT_EQUALS(executive.getStatus().toWord(), 0x8003);

		// The shared block and the program names stay in the client
		const uint sentBefore = host->sent.size();
		executive.setSharedData(fromHex("01 02"));
		executive.setNextProgram("SLand");
		executive.setCallbacks(true);
		TS_ASSERT(executive.getSharedData() == fromHex("01 02"));
		TS_ASSERT_EQUALS(host->sent.size(), sentBefore);
	}

	void test_lost_link_reads_as_carrier_lost_once() {
		LoopbackTransport *host = new LoopbackTransport();
		host->reply(kWelcome);
		host->reply(kConnected);
		Sci::OnlineTsnExecutive executive(host);
		TS_ASSERT_EQUALS(executive.connect("ATDT0"), Sci::kTsnConnected);

		TS_ASSERT_EQUALS(executive.poll(), Sci::kTsnLinkCarrierLost);
		TS_ASSERT_EQUALS(executive.service(), Sci::kTsnLinkOk);
		TS_ASSERT_EQUALS(executive.getStatus().connection, 0);
		TS_ASSERT_EQUALS(host->closes, 1);
	}

	void test_reply_with_another_tag_drops_the_link() {
		LoopbackTransport *host = new LoopbackTransport();
		host->reply(kWelcome);
		host->reply(kQueued);
		Sci::OnlineTsnExecutive executive(host);
		TS_ASSERT_EQUALS(executive.connect("ATDT0"), Sci::kTsnConnectNoCarrier);
		TS_ASSERT_EQUALS(host->closes, 1);
	}

	void test_stream_split_across_chunks() {
		LoopbackTransport *host = new LoopbackTransport();
		host->reply("0f 00");
		host->reply("00 00 02 00 00 01 0a 69 6e 6e 6b 65 65 70 65 72 64 05 00 00 00 11 01");
		host->reply("00 03 00");
		Sci::OnlineTsnExecutive executive(host);
		TS_ASSERT_EQUALS(executive.connect("ATDT0"), Sci::kTsnConnected);
	}

	void test_unwelcome_host_is_no_carrier() {
		LoopbackTransport *unreachable = new LoopbackTransport();
		unreachable->reachable = false;
		Sci::OnlineTsnExecutive offline(unreachable);
		TS_ASSERT_EQUALS(offline.connect("ATDT0"), Sci::kTsnConnectNoCarrier);

		LoopbackTransport *newer = new LoopbackTransport();
		newer->reply("0f 00 00 00 02 00 00 02 0a 69 6e 6e 6b 65 65 70 65 72 64");
		Sci::OnlineTsnExecutive mismatched(newer);
		TS_ASSERT_EQUALS(mismatched.connect("ATDT0"), Sci::kTsnConnectNoCarrier);
		TS_ASSERT_EQUALS(newer->closes, 1);
	}

	void test_unsendable_dial_string_is_not_sent() {
		LoopbackTransport *host = new LoopbackTransport();
		host->reply(kWelcome);
		Sci::OnlineTsnExecutive executive(host);
		TS_ASSERT_EQUALS(executive.connect("ATDT\x01"), Sci::kTsnConnectNoCarrier);
		TS_ASSERT(host->sent == fromHex(kHello));
	}
};
