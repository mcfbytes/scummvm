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

#ifndef SCI_ENGINE_TSN_OFFLINE_EXECUTIVE_H
#define SCI_ENGINE_TSN_OFFLINE_EXECUTIVE_H

#include "sci/engine/tsn_executive.h"

namespace Sci {

// An executive with a modem whose calls are never answered: every dial ends
// with no carrier, so the scripts take the error paths of a failed logon.
class OfflineTsnExecutive : public TsnExecutive {
public:
	TsnStatus getStatus() const override;
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
	bool isTransmitIdle() const override;
	uint16 switchHost(const Common::String &address) override;
	void flush() override;
	void setCallbacks(bool installed) override;
	uint16 getLineRate() const override;

private:
	Common::Array<byte> _sharedData;
};

} // End of namespace Sci

#endif // SCI_ENGINE_TSN_OFFLINE_EXECUTIVE_H
