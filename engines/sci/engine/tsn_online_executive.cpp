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

#include "common/debug.h"
#include "common/endian.h"

#include "sci/sci.h"
#include "sci/engine/tsn_envelope_parser.h"
#include "sci/engine/tsn_envelope_writer.h"
#include "sci/engine/tsn_online_executive.h"
#include "sci/engine/tsn_transport.h"

namespace Sci {

static const char *const kTsnClientName = "scummvm";

OnlineTsnExecutive::OnlineTsnExecutive(TsnTransport *transport) :
	_transport(transport),
	_linkOpen(false),
	_carrierLost(false),
	_nextTag(1) {
}

OnlineTsnExecutive::~OnlineTsnExecutive() {
	// Closing the stream ends the call on the host as a Disconnect does
	closeLink();
}

bool OnlineTsnExecutive::openLink() {
	_carrierLost = false;
	if (!_transport->open()) {
		warning("TSN executive: the host does not answer");
		return false;
	}

	Common::Array<byte> envelope;
	if (_transport->send(TsnEnvelopeWriter::makeHello(kTsnClientName).frame()) && receiveEnvelope(envelope)) {
		TsnEnvelopeParser welcome(envelope);
		const byte version = welcome.readByte();
		const Common::String server = welcome.readStr8();
		if (welcome.kind() == kTsnEnvelopeWelcome && welcome.isComplete() && version == kTsnTransportVersion) {
			debugC(kDebugLevelNetwork, "TSN executive: linked to %s", server.c_str());
			_linkOpen = true;
			return true;
		}
	}
	warning("TSN executive: the host does not speak version %d of the INT 14h transport", kTsnTransportVersion);
	closeLink();
	return false;
}

void OnlineTsnExecutive::closeLink() {
	_transport->close();
	_linkOpen = false;
	_inbox.clear();
}

void OnlineTsnExecutive::loseLink() {
	warning("TSN executive: lost the link to the host");
	closeLink();
	_carrierLost = true;
}

TsnEnvelopeWriter OnlineTsnExecutive::makeCall(TsnExport exportId) {
	return TsnEnvelopeWriter::makeCall(_nextTag++, exportId);
}

TsnEnvelopeParser OnlineTsnExecutive::exchange(const TsnEnvelopeWriter &call) {
	Common::Array<byte> envelope;
	if (!call.isValid() || !_transport->send(call.frame()) || !receiveEnvelope(envelope))
		envelope.clear();

	TsnEnvelopeParser reply(envelope);
	reply.require(reply.kind() == kTsnEnvelopeReply && reply.tag() == call.tag());
	reply.require(reply.readByte() == call.exportId());
	return reply;
}

bool OnlineTsnExecutive::receiveEnvelope(Common::Array<byte> &envelope) {
	for (;;) {
		const bool prefixed = _inbox.size() >= kTsnLengthPrefixSize;
		const uint32 size = prefixed ? READ_LE_UINT32(_inbox.data()) : 0;
		if (size > kTsnMaxEnvelopeSize)
			return false;

		const uint32 framedSize = kTsnLengthPrefixSize + size;
		if (prefixed && _inbox.size() >= framedSize) {
			envelope = Common::Array<byte>(_inbox.data() + kTsnLengthPrefixSize, size);
			_inbox = Common::Array<byte>(_inbox.data() + framedSize, _inbox.size() - framedSize);
			return true;
		}
		if (!_transport->receive(_inbox))
			return false;
	}
}

bool OnlineTsnExecutive::finish(const TsnEnvelopeParser &reply) {
	if (reply.isComplete())
		return true;
	loseLink();
	return false;
}

bool OnlineTsnExecutive::callWithoutResult(TsnExport exportId) {
	return finish(exchange(makeCall(exportId)));
}

TsnLinkStatus OnlineTsnExecutive::queryLinkStatus(TsnExport exportId) {
	if (_linkOpen) {
		TsnEnvelopeParser reply = exchange(makeCall(exportId));
		const byte status = reply.readByte();
		reply.require(status <= kTsnLinkTimeoutLimit);
		if (finish(reply))
			return (TsnLinkStatus)status;
	}
	// A lost link reads as a lost carrier once, as a modem hang-up does
	const bool lost = _carrierLost;
	_carrierLost = false;
	return lost ? kTsnLinkCarrierLost : kTsnLinkOk;
}

TsnStatus OnlineTsnExecutive::getStatus() {
	if (!_linkOpen)
		return _onHook.getStatus();

	TsnEnvelopeParser reply = exchange(makeCall(kTsnExportGetStatus));
	TsnStatus status;
	status.driver = reply.readByte();
	status.connection = reply.readByte();
	return finish(reply) ? status : _onHook.getStatus();
}

Common::Array<byte> OnlineTsnExecutive::getSharedData() const {
	return _onHook.getSharedData();
}

void OnlineTsnExecutive::setSharedData(const Common::Array<byte> &block) {
	_onHook.setSharedData(block);
}

uint16 OnlineTsnExecutive::connect(const Common::String &dialString) {
	if (!_linkOpen && !openLink())
		return _onHook.connect(dialString);

	TsnEnvelopeWriter call = makeCall(kTsnExportConnect);
	call.writeStr8(dialString, kTsnMaxDialStringSize);
	TsnEnvelopeParser reply = exchange(call);
	const byte result = reply.readByte();
	return finish(reply) ? result : (uint16)kTsnConnectNoCarrier;
}

bool OnlineTsnExecutive::send(const Common::Array<byte> &body) {
	if (!_linkOpen)
		return _onHook.send(body);

	TsnEnvelopeWriter call = makeCall(kTsnExportSend);
	call.writeBytes16(body);
	TsnEnvelopeParser reply = exchange(call);
	const bool queued = reply.readBool();
	return finish(reply) && queued;
}

bool OnlineTsnExecutive::receive(Common::Array<byte> &body) {
	if (!_linkOpen)
		return _onHook.receive(body);

	TsnEnvelopeParser reply = exchange(makeCall(kTsnExportReceive));
	const bool waiting = reply.readBool();
	if (waiting)
		body = reply.readBytes16();
	return finish(reply) && waiting;
}

void OnlineTsnExecutive::setAckTimeout(uint16 ticks) {
	_onHook.setAckTimeout(ticks);
}

void OnlineTsnExecutive::disconnect() {
	if (_linkOpen && callWithoutResult(kTsnExportDisconnect))
		closeLink();
	_carrierLost = false;
}

TsnLinkStatus OnlineTsnExecutive::poll() {
	return queryLinkStatus(kTsnExportPoll);
}

void OnlineTsnExecutive::setNextProgram(const Common::String &name) {
	_onHook.setNextProgram(name);
}

TsnLinkStatus OnlineTsnExecutive::service() {
	return queryLinkStatus(kTsnExportService);
}

Common::String OnlineTsnExecutive::getPreviousProgram() const {
	return _onHook.getPreviousProgram();
}

bool OnlineTsnExecutive::isTransmitIdle() {
	if (!_linkOpen)
		return _onHook.isTransmitIdle();

	TsnEnvelopeParser reply = exchange(makeCall(kTsnExportIsTransmitIdle));
	const bool idle = reply.readBool();
	return !finish(reply) || idle;
}

uint16 OnlineTsnExecutive::switchHost(const Common::String &address) {
	if (!_linkOpen)
		return _onHook.switchHost(address);

	TsnEnvelopeWriter call = makeCall(kTsnExportSwitchHost);
	call.writeStr8(address, kTsnMaxDialStringSize);
	TsnEnvelopeParser reply = exchange(call);
	const byte result = reply.readByte();
	return finish(reply) ? result : (uint16)kTsnSwitchNotConnected;
}

void OnlineTsnExecutive::flush() {
	if (_linkOpen)
		callWithoutResult(kTsnExportFlush);
}

void OnlineTsnExecutive::setCallbacks(bool installed) {
	_onHook.setCallbacks(installed);
}

uint16 OnlineTsnExecutive::getLineRate() {
	if (!_linkOpen)
		return _onHook.getLineRate();

	TsnEnvelopeParser reply = exchange(makeCall(kTsnExportGetLineRate));
	const uint16 rate = reply.readUint16();
	return finish(reply) ? rate : _onHook.getLineRate();
}

} // End of namespace Sci
