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

#include "sci/engine/tsn_session.h"

namespace Sci {

enum {
	kTsnAckTimeoutTicks = 300,
	kTsnServicePeriodTicks = 20,
	kTsnPollHookSlots = 5
};

TsnSession::TsnSession(TsnExecutive *executive) :
	_executive(executive),
	_bound(false),
	_pollHooks(0),
	_serverInstalled(false),
	_serverCountdown(0),
	_clock(0) {
}

TsnSession::~TsnSession() {
	// LSCITV unbinds on exit, which tells the executive that no interpreter is left
	if (_bound)
		_executive->setCallbacks(false);
}

TsnExecutive &TsnSession::executive() {
	if (!_bound) {
		_executive->setAckTimeout(kTsnAckTimeoutTicks);
		_executive->setCallbacks(true);
		_bound = true;
	}
	return *_executive;
}

bool TsnSession::connect(const Common::String &dialString) {
	if (executive().getStatus().connection)
		return true;

	const uint16 result = _executive->connect(dialString);
	if (result == kTsnConnected)
		installServer();
	else
		postError(result);
	addPollHook();
	return result == kTsnConnected;
}

bool TsnSession::switchHost(const Common::String &address) {
	const uint16 result = executive().switchHost(address);
	removePollHook();
	if (result == kTsnConnected) {
		_serverInstalled = false;
		installServer();
	} else {
		postError(result);
	}
	addPollHook();
	return result == kTsnConnected;
}

void TsnSession::disconnect() {
	_serverInstalled = false;
	removePollHook();
	executive().disconnect();
}

void TsnSession::advanceClock(uint32 tick) {
	const uint32 elapsed = tick - _clock;
	_clock = tick;
	if (!_serverInstalled)
		return;

	if (elapsed < _serverCountdown) {
		_serverCountdown -= elapsed;
		return;
	}
	_serverCountdown = kTsnServicePeriodTicks;
	postError(_executive->service());
}

void TsnSession::runPollHooks() {
	for (uint hook = 0; hook < _pollHooks; ++hook)
		pump();
}

void TsnSession::pump() {
	postError(executive().poll());

	Common::Array<byte> body;
	while (_executive->receive(body))
		_messages.push_back(body);
}

void TsnSession::postError(uint16 code) {
	if (code)
		_errors.push_back(code);
}

void TsnSession::addPollHook() {
	// The hook table takes duplicates and silently drops an add when it is full
	if (_pollHooks < kTsnPollHookSlots)
		++_pollHooks;
}

void TsnSession::removePollHook() {
	if (_pollHooks)
		--_pollHooks;
}

void TsnSession::installServer() {
	// The server table ignores duplicates, which keeps the running countdown
	if (!_serverInstalled) {
		_serverInstalled = true;
		_serverCountdown = kTsnServicePeriodTicks;
	}
}

} // End of namespace Sci
