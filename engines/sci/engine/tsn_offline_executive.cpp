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

#include "sci/sci.h"
#include "sci/engine/tsn_offline_executive.h"

namespace Sci {

enum {
	kModemDriverId = 3,
	kModemInitialLineRate = 1200
};

TsnStatus OfflineTsnExecutive::getStatus() {
	const TsnStatus status = { kModemDriverId, 0 };
	return status;
}

Common::Array<byte> OfflineTsnExecutive::getSharedData() const {
	return _sharedData;
}

void OfflineTsnExecutive::setSharedData(const Common::Array<byte> &block) {
	const uint size = MIN<uint>(block.size(), kTsnSharedDataSize);
	_sharedData = Common::Array<byte>(block.data(), size);
}

uint16 OfflineTsnExecutive::connect(const Common::String &dialString) {
	debugC(kDebugLevelNetwork, "TSN executive: no carrier on dial string '%s'", dialString.c_str());
	return kTsnConnectNoCarrier;
}

bool OfflineTsnExecutive::send(const Common::Array<byte> &body) {
	return false;
}

bool OfflineTsnExecutive::receive(Common::Array<byte> &body) {
	return false;
}

void OfflineTsnExecutive::setAckTimeout(uint16 ticks) {
	// No link layer, so nothing is ever retransmitted
}

void OfflineTsnExecutive::disconnect() {
}

TsnLinkStatus OfflineTsnExecutive::poll() {
	return kTsnLinkOk;
}

void OfflineTsnExecutive::setNextProgram(const Common::String &name) {
	debugC(kDebugLevelNetwork, "TSN executive: next program '%s'", name.c_str());
}

TsnLinkStatus OfflineTsnExecutive::service() {
	return kTsnLinkOk;
}

Common::String OfflineTsnExecutive::getPreviousProgram() const {
	return Common::String();
}

bool OfflineTsnExecutive::isTransmitIdle() {
	return true;
}

uint16 OfflineTsnExecutive::switchHost(const Common::String &address) {
	return kTsnSwitchNotConnected;
}

void OfflineTsnExecutive::flush() {
}

void OfflineTsnExecutive::setCallbacks(bool installed) {
	// Message bodies are copied, so there are no handles to call back for
}

uint16 OfflineTsnExecutive::getLineRate() {
	return kModemInitialLineRate;
}

} // End of namespace Sci
