/* ScummVM - Graphic Adventure Engine
 *
 * ScummVM is the legal property of its developers, whose names
 * are too numerous to list here. Please refer to the COPYRIGHT
 * file distributed with this source distribution.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "sci/sci.h"
#include "sci/event.h"
#include "sci/engine/kernel.h"
#include "sci/engine/lsci_array.h"
#include "sci/engine/lsci_kernel.h"
#include "sci/engine/selector.h"
#include "sci/engine/state.h"
#include "sci/engine/tsn_session.h"

namespace Sci {

// Event types that LSCITV adds for the network; the error type shares its bit
// with ScummVM's quit event, which never reaches the scripts.
enum {
	kLsciEventNetworkMessage = 0x400,
	kLsciEventNetworkError = 0x800
};

static TsnSession &tsnSession() {
	TsnSession &session = *g_sci->getTsnSession();
	session.advanceClock(g_sci->getTickCount());
	return session;
}

static TsnExecutive &tsnExecutive() {
	return tsnSession().executive();
}

static void writeEvent(SegManager *segMan, reg_t event, uint16 type, reg_t message, uint16 modifiers) {
	writeSelectorValue(segMan, event, SELECTOR(type), type);
	writeSelector(segMan, event, SELECTOR(message), message);
	writeSelectorValue(segMan, event, SELECTOR(modifiers), modifiers);
}

static void appendWord(Common::Array<byte> &body, uint16 word) {
	body.push_back(word & 0xFF);
	body.push_back(word >> 8);
}

// The formatted send of TSN(4), codes b, w, s, a and + as docs/protocol/ktsn.md lists them
static Common::Array<byte> formatTsnMessage(SegManager *segMan, const Common::String &format, int argc, const reg_t *argv) {
	Common::Array<byte> body;
	int next = 0;
	char previous = 0;
	for (uint i = 0; i < format.size(); ++i) {
		const char code = format[i];
		if (code == '+') {
			for (; next < argc; ++next) {
				if (previous == 'b')
					body.push_back(argv[next].getOffset() & 0xFF);
				else
					appendWord(body, argv[next].getOffset());
			}
			continue;
		}
		previous = code;

		const reg_t value = next < argc ? argv[next++] : NULL_REG;
		switch (code) {
		case 'b':
			body.push_back(value.getOffset() & 0xFF);
			break;
		case 'w':
			appendWord(body, value.getOffset());
			break;
		case 's': {
			const Common::String text = getLsciString(segMan, value);
			body.push_back(Common::Array<byte>((const byte *)text.c_str(), text.size() + 1));
			break;
		}
		case 'a': {
			const reg_t array = next < argc ? argv[next++] : NULL_REG;
			const uint byteCount = value.toUint16() * getLsciElementSize(segMan, array);
			Common::Array<byte> bytes = getLsciArrayBytes(segMan, array, byteCount);
			// LSCITV copies the count whatever the array's size; the stock Login has 0 past the end
			bytes.resize(byteCount);
			body.push_back(bytes);
			break;
		}
		default:
			break;
		}
	}
	return body;
}

// Receive hands each body over as a byte array, which the event then carries
static void postNetworkMessages(EngineState *s, TsnSession &session) {
	for (; session.hasMessage(); session.dropMessage())
		s->_lsciNetworkMessages.push_back(makeLsciByteArray(s->_segMan, session.frontMessage()));
}

reg_t kLsciGetEvent(EngineState *s, int argc, reg_t *argv) {
	TsnSession &session = tsnSession();
	session.runPollHooks();
	postNetworkMessages(s, session);

	const uint16 mask = argv[0].toUint16();
	const bool peek = mask & kSciEventPeek;
	const reg_t event = argv[1];
	Common::Array<reg_t> &messages = s->_lsciNetworkMessages;
	if ((mask & kLsciEventNetworkMessage) && !messages.empty()) {
		const reg_t message = messages.front();
		writeEvent(s->_segMan, event, kLsciEventNetworkMessage, message, s->_segMan->lookupArray(message)->size());
		if (!peek)
			messages.remove_at(0);
		return TRUE_REG;
	}
	if ((mask & ~kSciEventPeek) == kLsciEventNetworkMessage) {
		writeEvent(s->_segMan, event, kSciEventNone, NULL_REG, 0);
		return NULL_REG;
	}
	if ((mask & kLsciEventNetworkError) && session.hasError()) {
		writeEvent(s->_segMan, event, kLsciEventNetworkError, NULL_REG, session.frontError());
		if (!peek)
			session.dropError();
		return TRUE_REG;
	}
	return kGetEvent(s, argc, argv);
}

static reg_t kTsnGetStatus(EngineState *s, int argc, reg_t *argv) {
	return make_reg(0, tsnExecutive().getStatus().toWord());
}

static reg_t kTsnGetSharedData(EngineState *s, int argc, reg_t *argv) {
	const Common::Array<byte> block = tsnExecutive().getSharedData();
	return block.empty() ? NULL_REG : makeLsciByteArray(s->_segMan, block);
}

static reg_t kTsnSetSharedData(EngineState *s, int argc, reg_t *argv) {
	Common::Array<byte> block;
	if (!argv[0].isNull() && argv[1].toUint16())
		block = getLsciArrayBytes(s->_segMan, argv[0], argv[1].toUint16());
	tsnExecutive().setSharedData(block);
	return s->r_acc;
}

static reg_t kTsnConnect(EngineState *s, int argc, reg_t *argv) {
	return make_reg(0, tsnSession().connect(getLsciString(s->_segMan, argv[0])));
}

static reg_t kTsnSend(EngineState *s, int argc, reg_t *argv) {
	TsnExecutive &executive = tsnExecutive();
	const Common::String format = getLsciString(s->_segMan, argv[0]);
	const Common::Array<byte> body = formatTsnMessage(s->_segMan, format, argc - 1, argv + 1);
	if (body.empty())
		return NULL_REG;
	return make_reg(0, executive.send(body));
}

static reg_t kTsnReserved(EngineState *s, int argc, reg_t *argv) {
	// Binds the executive like every sub-op, then leaves the accumulator alone
	tsnExecutive();
	return s->r_acc;
}

static reg_t kTsnDisconnect(EngineState *s, int argc, reg_t *argv) {
	tsnSession().disconnect();
	return s->r_acc;
}

static reg_t kTsnPoll(EngineState *s, int argc, reg_t *argv) {
	TsnSession &session = tsnSession();
	session.pump();
	postNetworkMessages(s, session);
	return s->r_acc;
}

static reg_t kTsnSetNextProgram(EngineState *s, int argc, reg_t *argv) {
	const Common::String name = argv[0].isNull() ? Common::String() : getLsciString(s->_segMan, argv[0]);
	tsnExecutive().setNextProgram(name);
	return s->r_acc;
}

static reg_t kTsnGetPreviousProgram(EngineState *s, int argc, reg_t *argv) {
	const Common::String name = tsnExecutive().getPreviousProgram();
	return name.empty() ? NULL_REG : makeLsciString(s->_segMan, name);
}

static reg_t kTsnIsTransmitIdle(EngineState *s, int argc, reg_t *argv) {
	return make_reg(0, tsnExecutive().isTransmitIdle());
}

static reg_t kTsnSwitchHost(EngineState *s, int argc, reg_t *argv) {
	return make_reg(0, tsnSession().switchHost(getLsciString(s->_segMan, argv[0])));
}

static reg_t kTsnFlush(EngineState *s, int argc, reg_t *argv) {
	tsnExecutive().flush();
	return s->r_acc;
}

static reg_t kTsnGetLineRate(EngineState *s, int argc, reg_t *argv) {
	return make_reg(0, tsnExecutive().getLineRate());
}

// LSCITV's jump table: sub-op n calls export n, and a sub-op past the table does nothing
static KernelFunctionCall *const s_tsnSubops[kTsnExportCount] = {
	kTsnGetStatus,
	kTsnGetSharedData,
	kTsnSetSharedData,
	kTsnConnect,
	kTsnSend,
	kTsnReserved,
	kTsnReserved,
	kTsnDisconnect,
	kTsnPoll,
	kTsnSetNextProgram,
	kTsnReserved,
	kTsnGetPreviousProgram,
	kTsnIsTransmitIdle,
	kTsnSwitchHost,
	kTsnFlush,
	kTsnReserved,
	kTsnGetLineRate
};

reg_t kTsn(EngineState *s, int argc, reg_t *argv) {
	const uint16 subop = argv[0].toUint16();
	KernelFunctionCall *const call = subop < kTsnExportCount ? s_tsnSubops[subop] : kTsnReserved;
	return call(s, argc - 1, argv + 1);
}

} // End of namespace Sci
