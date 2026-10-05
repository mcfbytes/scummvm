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

#ifndef SCI_ENGINE_TSN_EXECUTIVE_H
#define SCI_ENGINE_TSN_EXECUTIVE_H

#include "common/array.h"
#include "common/str.h"

namespace Sci {

// Status of the link, from Poll and Service
enum TsnLinkStatus {
	kTsnLinkOk = 0,
	kTsnLinkCarrierLost = 1,
	kTsnLinkNakLimit = 2,
	kTsnLinkTimeoutLimit = 3
};

// Connect and SwitchHost results; the remaining codes come from the com driver
enum TsnConnectResult {
	kTsnConnected = 0,
	kTsnSwitchNotConnected = 1,
	kTsnConnectNoCarrier = 10
};

// The executive's exports; a TSN sub-op is the number of the export it calls
enum TsnExport {
	kTsnExportGetStatus = 0,
	kTsnExportGetSharedData = 1,
	kTsnExportSetSharedData = 2,
	kTsnExportConnect = 3,
	kTsnExportSend = 4,
	kTsnExportReceive = 5,
	kTsnExportSetAckTimeout = 6,
	kTsnExportDisconnect = 7,
	kTsnExportPoll = 8,
	kTsnExportSetNextProgram = 9,
	kTsnExportService = 10,
	kTsnExportGetPreviousProgram = 11,
	kTsnExportIsTransmitIdle = 12,
	kTsnExportSwitchHost = 13,
	kTsnExportFlush = 14,
	kTsnExportSetCallbacks = 15,
	kTsnExportGetLineRate = 16,
	kTsnExportCount = 17
};

enum {
	kTsnConnectionUp = 0x80,
	kTsnSharedDataSize = 256
};

struct TsnStatus {
	byte driver;     // com driver id: MODEM.DRV reports 3
	byte connection; // kTsnConnectionUp while connected, 0 otherwise

	uint16 toWord() const { return driver | (connection << 8); }
};

// The 17 exports of the resident TSN executive (TSNEXEC), in export order.
// Semantics: docs/protocol/int14h-api.md; an empty program name stands for NULL.
class TsnExecutive {
public:
	virtual ~TsnExecutive() {}

	virtual TsnStatus getStatus() = 0;
	virtual Common::Array<byte> getSharedData() const = 0;
	virtual void setSharedData(const Common::Array<byte> &block) = 0;
	virtual uint16 connect(const Common::String &dialString) = 0;
	virtual bool send(const Common::Array<byte> &body) = 0;
	virtual bool receive(Common::Array<byte> &body) = 0;
	virtual void setAckTimeout(uint16 ticks) = 0;
	virtual void disconnect() = 0;
	virtual TsnLinkStatus poll() = 0;
	virtual void setNextProgram(const Common::String &name) = 0;
	virtual TsnLinkStatus service() = 0;
	virtual Common::String getPreviousProgram() const = 0;
	virtual bool isTransmitIdle() = 0;
	virtual uint16 switchHost(const Common::String &address) = 0;
	virtual void flush() = 0;
	virtual void setCallbacks(bool installed) = 0;
	virtual uint16 getLineRate() = 0;
};

// The executive that the game options select: innkeeperd at lsci_host, else offline
TsnExecutive *makeTsnExecutive();

} // End of namespace Sci

#endif // SCI_ENGINE_TSN_EXECUTIVE_H
