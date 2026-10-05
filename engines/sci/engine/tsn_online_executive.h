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

#ifndef SCI_ENGINE_TSN_ONLINE_EXECUTIVE_H
#define SCI_ENGINE_TSN_ONLINE_EXECUTIVE_H

#include "common/ptr.h"

#include "sci/engine/tsn_envelope.h"
#include "sci/engine/tsn_offline_executive.h"

namespace Sci {

class TsnEnvelopeParser;
class TsnEnvelopeWriter;
class TsnTransport;

// An executive whose modem is a link to innkeeperd over the INT 14h transport
// (docs/protocol/int14h-transport.md). Connect opens the link, Disconnect closes it.
class OnlineTsnExecutive : public TsnExecutive {
public:
	explicit OnlineTsnExecutive(TsnTransport *transport);
	~OnlineTsnExecutive() override;

	TsnStatus getStatus() override;
	Common::Array<byte> getSharedData() const override;
	void setSharedData(const Common::Array<byte> &block) override;
	uint16 connect(const Common::String &dialString) override;
	bool send(const Common::Array<byte> &body) override;
	bool receive(Common::Array<byte> &body) override;
	void setAckTimeout(uint16 ticks) override;
	void disconnect() override;
	TsnLinkStatus poll() override;
	void setNextProgram(const Common::String &name) override;
	TsnLinkStatus service() override;
	Common::String getPreviousProgram() const override;
	bool isTransmitIdle() override;
	uint16 switchHost(const Common::String &address) override;
	void flush() override;
	void setCallbacks(bool installed) override;
	uint16 getLineRate() override;

private:
	bool openLink();
	void closeLink();
	void loseLink();
	TsnEnvelopeWriter makeCall(TsnExport exportId);
	TsnEnvelopeParser exchange(const TsnEnvelopeWriter &call);
	bool receiveEnvelope(Common::Array<byte> &envelope);
	bool finish(const TsnEnvelopeParser &reply);
	bool callWithoutResult(TsnExport exportId);
	TsnLinkStatus queryLinkStatus(TsnExport exportId);

	Common::ScopedPtr<TsnTransport> _transport;
	// Keeps the executive-local state and answers for the line while no link is open
	OfflineTsnExecutive _onHook;
	bool _linkOpen;
	bool _carrierLost;
	uint16 _nextTag;
	Common::Array<byte> _inbox;
};

} // End of namespace Sci

#endif // SCI_ENGINE_TSN_ONLINE_EXECUTIVE_H
