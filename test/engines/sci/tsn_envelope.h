#include <cxxtest/TestSuite.h>

#include "sci/engine/tsn_envelope_parser.h"
#include "sci/engine/tsn_envelope_writer.h"

namespace {

// Bytes of crates/int14h/tests/golden/envelopes.txt, length prefix included
const byte kGoldenHello[] = { 0x10, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x49, 0x31, 0x34, 0x48, 0x01, 0x07, 0x73, 0x63, 0x75, 0x6d, 0x6d, 0x76, 0x6d };
const byte kGoldenConnect[] = { 0x0a, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x03, 0x05, 0x41, 0x54, 0x44, 0x54, 0x30 };
const byte kGoldenSend[] = { 0x09, 0x00, 0x00, 0x00, 0x10, 0x02, 0x00, 0x04, 0x03, 0x00, 0x22, 0x04, 0x10 };
const byte kGoldenReceived[] = { 0x11, 0x04, 0x00, 0x05, 0x01, 0x02, 0x00, 0x07, 0x41 };
const byte kGoldenStatus[] = { 0x11, 0x07, 0x00, 0x00, 0x03, 0x80 };

Common::Array<byte> bytesOf(const byte *data, uint size) {
	return Common::Array<byte>(data, size);
}

} // End of anonymous namespace

class TsnEnvelopeTestSuite : public CxxTest::TestSuite {
public:
	void test_writes_golden_frames() {
		TS_ASSERT(Sci::TsnEnvelopeWriter::makeHello("scummvm").frame() == bytesOf(kGoldenHello, sizeof(kGoldenHello)));

		Sci::TsnEnvelopeWriter connect = Sci::TsnEnvelopeWriter::makeCall(1, Sci::kTsnExportConnect);
		connect.writeStr8("ATDT0", Sci::kTsnMaxDialStringSize);
		TS_ASSERT(connect.isValid());
		TS_ASSERT(connect.frame() == bytesOf(kGoldenConnect, sizeof(kGoldenConnect)));

		Sci::TsnEnvelopeWriter send = Sci::TsnEnvelopeWriter::makeCall(2, Sci::kTsnExportSend);
		const byte body[] = { 0x22, 0x04, 0x10 };
		send.writeBytes16(bytesOf(body, sizeof(body)));
		TS_ASSERT(send.frame() == bytesOf(kGoldenSend, sizeof(kGoldenSend)));
	}

	void test_rejects_fields_the_transport_cannot_carry() {
		Sci::TsnEnvelopeWriter control = Sci::TsnEnvelopeWriter::makeCall(1, Sci::kTsnExportConnect);
		control.writeStr8("ATDT\x01");
		TS_ASSERT(!control.isValid());

		Sci::TsnEnvelopeWriter tooLong = Sci::TsnEnvelopeWriter::makeCall(1, Sci::kTsnExportSwitchHost);
		tooLong.writeStr8(Common::String(128, 'A'), Sci::kTsnMaxDialStringSize);
		TS_ASSERT(!tooLong.isValid());
	}

	void test_parses_golden_replies() {
		Sci::TsnEnvelopeParser received(bytesOf(kGoldenReceived, sizeof(kGoldenReceived)));
		TS_ASSERT_EQUALS(received.kind(), Sci::kTsnEnvelopeReply);
		TS_ASSERT_EQUALS(received.tag(), 4);
		TS_ASSERT_EQUALS(received.readByte(), Sci::kTsnExportReceive);
		TS_ASSERT(received.readBool());
		const Common::Array<byte> body = received.readBytes16();
		TS_ASSERT_EQUALS(body.size(), 2U);
		TS_ASSERT_EQUALS(body[1], 0x41);
		TS_ASSERT(received.isComplete());

		Sci::TsnEnvelopeParser status(bytesOf(kGoldenStatus, sizeof(kGoldenStatus)));
		TS_ASSERT_EQUALS(status.tag(), 7);
		status.readByte();
		TS_ASSERT_EQUALS(status.readByte(), 3);
		TS_ASSERT_EQUALS(status.readByte(), 0x80);
		TS_ASSERT(status.isComplete());
	}

	void test_rejects_malformed_replies() {
		const byte badBool[] = { 0x11, 0x01, 0x00, 0x04, 0x02 };
		Sci::TsnEnvelopeParser parsedBool(bytesOf(badBool, sizeof(badBool)));
		parsedBool.readByte();
		parsedBool.readBool();
		TS_ASSERT(!parsedBool.isComplete());

		const byte leftover[] = { 0x11, 0x01, 0x00, 0x08, 0x00, 0x00 };
		Sci::TsnEnvelopeParser parsedLeftover(bytesOf(leftover, sizeof(leftover)));
		parsedLeftover.readByte();
		parsedLeftover.readByte();
		TS_ASSERT(!parsedLeftover.isComplete());

		const byte shortRun[] = { 0x11, 0x01, 0x00, 0x05, 0x01, 0x05, 0x00, 0x07 };
		Sci::TsnEnvelopeParser parsedRun(bytesOf(shortRun, sizeof(shortRun)));
		parsedRun.readByte();
		parsedRun.readBool();
		TS_ASSERT(parsedRun.readBytes16().empty());
		TS_ASSERT(!parsedRun.isComplete());
	}
};
