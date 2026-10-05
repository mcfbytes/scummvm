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

#ifndef SCI_ENGINE_TSN_SESSION_H
#define SCI_ENGINE_TSN_SESSION_H

#include "common/list.h"
#include "common/ptr.h"

#include "sci/engine/tsn_executive.h"

namespace Sci {

// LSCITV's side of the executive: binding on first use, the GetEvent poll hooks,
// the timer server and the queued network events (docs/protocol/ktsn.md).
class TsnSession {
public:
	explicit TsnSession(TsnExecutive *executive);
	~TsnSession();

	TsnExecutive &executive();

	bool connect(const Common::String &dialString);
	bool switchHost(const Common::String &address);
	void disconnect();

	void advanceClock(uint32 tick);
	void runPollHooks();
	void pump();

	bool hasMessage() const { return !_messages.empty(); }
	const Common::Array<byte> &frontMessage() const { return _messages.front(); }
	void dropMessage() { _messages.pop_front(); }

	bool hasError() const { return !_errors.empty(); }
	uint16 frontError() const { return _errors.front(); }
	void dropError() { _errors.pop_front(); }

private:
	void postError(uint16 code);
	void addPollHook();
	void removePollHook();
	void installServer();

	Common::ScopedPtr<TsnExecutive> _executive;
	bool _bound;
	uint _pollHooks;
	bool _serverInstalled;
	uint32 _serverCountdown;
	uint32 _clock;
	Common::List<Common::Array<byte> > _messages;
	Common::List<uint16> _errors;
};

} // End of namespace Sci

#endif // SCI_ENGINE_TSN_SESSION_H
