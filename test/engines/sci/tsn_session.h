#include <cxxtest/TestSuite.h>

#include "sci/engine/tsn_offline_executive.h"
#include "sci/engine/tsn_session.h"

namespace {

// An executive that records what the session asks of it and replays set answers
class FakeTsnExecutive : public Sci::TsnExecutive {
public:
	FakeTsnExecutive(int *callbackCount) : callbacks(callbackCount), ackTimeout(0), connectResult(Sci::kTsnConnected),
		connected(false), polls(0), services(0), pollResult(Sci::kTsnLinkOk), serviceResult(Sci::kTsnLinkOk) {}

	Sci::TsnStatus getStatus() override {
		const Sci::TsnStatus status = { 3, (byte)(connected ? Sci::kTsnConnectionUp : 0) };
		return status;
	}
	Common::Array<byte> getSharedData() const override { return Common::Array<byte>(); }
	void setSharedData(const Common::Array<byte> &block) override {}
	uint16 connect(const Common::String &dialString) override {
		connected = connectResult == Sci::kTsnConnected;
		return connectResult;
	}
	bool send(const Common::Array<byte> &body) override { return true; }
	bool receive(Common::Array<byte> &body) override {
		if (incoming.empty())
			return false;
		body = incoming.front();
		incoming.pop_front();
		return true;
	}
	void setAckTimeout(uint16 ticks) override { ackTimeout = ticks; }
	void disconnect() override { connected = false; }
	Sci::TsnLinkStatus poll() override { ++polls; return pollResult; }
	void setNextProgram(const Common::String &name) override {}
	Sci::TsnLinkStatus service() override { ++services; return serviceResult; }
	Common::String getPreviousProgram() const override { return Common::String(); }
	bool isTransmitIdle() override { return true; }
	uint16 switchHost(const Common::String &address) override { return Sci::kTsnConnected; }
	void flush() override {}
	void setCallbacks(bool installed) override { *callbacks += installed ? 1 : -1; }
	uint16 getLineRate() override { return 2400; }

	int *callbacks;
	uint16 ackTimeout;
	uint16 connectResult;
	bool connected;
	int polls;
	int services;
	Sci::TsnLinkStatus pollResult;
	Sci::TsnLinkStatus serviceResult;
	Common::List<Common::Array<byte> > incoming;
};

} // End of anonymous namespace

class TsnSessionTestSuite : public CxxTest::TestSuite {
public:
	void test_binds_on_first_use_and_unbinds() {
		int callbacks = 0;
		FakeTsnExecutive *fake = new FakeTsnExecutive(&callbacks);
		{
			Sci::TsnSession session(fake);
			TS_ASSERT_EQUALS(callbacks, 0);
			session.executive();
			session.executive();
			TS_ASSERT_EQUALS(callbacks, 1);
			TS_ASSERT_EQUALS(fake->ackTimeout, 300);
		}
		TS_ASSERT_EQUALS(callbacks, 0);
	}

	void test_offline_dial_ends_without_carrier() {
		Sci::TsnSession session(new Sci::OfflineTsnExecutive());
		TS_ASSERT_EQUALS(session.executive().getStatus().toWord(), 3);
		TS_ASSERT_EQUALS(session.executive().getLineRate(), 1200);
		TS_ASSERT(!session.connect("555-1234"));
		TS_ASSERT(session.hasError());
		TS_ASSERT_EQUALS(session.frontError(), Sci::kTsnConnectNoCarrier);
		session.dropError();
		TS_ASSERT(!session.hasError());
		TS_ASSERT_EQUALS(session.executive().switchHost("SIERRA"), Sci::kTsnSwitchNotConnected);
	}

	void test_connection_services_every_twenty_ticks() {
		int callbacks = 0;
		FakeTsnExecutive *fake = new FakeTsnExecutive(&callbacks);
		Sci::TsnSession session(fake);
		session.advanceClock(100);
		TS_ASSERT(session.connect("555-1234"));
		session.advanceClock(119);
		TS_ASSERT_EQUALS(fake->services, 0);
		session.advanceClock(120);
		TS_ASSERT_EQUALS(fake->services, 1);
		fake->serviceResult = Sci::kTsnLinkCarrierLost;
		session.advanceClock(140);
		TS_ASSERT_EQUALS(fake->services, 2);
		TS_ASSERT(session.hasError());
		TS_ASSERT_EQUALS(session.frontError(), Sci::kTsnLinkCarrierLost);
		session.disconnect();
		session.advanceClock(200);
		TS_ASSERT_EQUALS(fake->services, 2);
	}

	void test_poll_hooks_queue_messages() {
		int callbacks = 0;
		FakeTsnExecutive *fake = new FakeTsnExecutive(&callbacks);
		Sci::TsnSession session(fake);
		session.runPollHooks();
		TS_ASSERT_EQUALS(fake->polls, 0);

		TS_ASSERT(session.connect("555-1234"));
		// Already connected: the hook is not added a second time
		TS_ASSERT(session.connect("555-1234"));
		const byte first[] = { 1, 2, 3 };
		const byte second[] = { 4 };
		fake->incoming.push_back(Common::Array<byte>(first, sizeof(first)));
		fake->incoming.push_back(Common::Array<byte>(second, sizeof(second)));
		session.runPollHooks();
		TS_ASSERT_EQUALS(fake->polls, 1);
		TS_ASSERT(session.hasMessage());
		TS_ASSERT_EQUALS(session.frontMessage().size(), 3U);
		session.dropMessage();
		TS_ASSERT_EQUALS(session.frontMessage()[0], 4);
		session.dropMessage();
		TS_ASSERT(!session.hasMessage());
	}
};
