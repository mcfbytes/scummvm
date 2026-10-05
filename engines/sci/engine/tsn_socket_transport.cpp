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

#include "backends/networking/basic/socket.h"
#include "common/system.h"

#include "sci/engine/tsn_socket_transport.h"

namespace Sci {

enum {
	kTsnReplyTimeoutMillis = 10000,
	kTsnReceiveChunkSize = 4096
};

// Told to connect only, libcurl opens a plain TCP connection for an http URL
// and sends nothing, which the SCUMM HE lobby relies on too.
TsnSocketTransport::TsnSocketTransport(const Common::String &host, int port) :
	_url(Common::String::format("http://%s:%d", host.c_str(), port)) {
}

TsnSocketTransport::~TsnSocketTransport() {
}

bool TsnSocketTransport::open() {
	_socket.reset(Networking::Socket::connect(_url));
	return _socket.get() != nullptr;
}

void TsnSocketTransport::close() {
	_socket.reset();
}

bool TsnSocketTransport::send(const Common::Array<byte> &bytes) {
	return _socket.get() && _socket->send((const char *)bytes.data(), bytes.size()) == bytes.size();
}

bool TsnSocketTransport::receive(Common::Array<byte> &bytes) {
	const uint32 deadline = g_system->getMillis() + kTsnReplyTimeoutMillis;
	while (_socket.get()) {
		const int ready = _socket->ready();
		if (ready < 0)
			return false;
		if (ready > 0) {
			byte chunk[kTsnReceiveChunkSize];
			const size_t size = _socket->recv(chunk, sizeof(chunk));
			bytes.push_back(Common::Array<byte>(chunk, size));
			return size > 0;
		}
		if (g_system->getMillis() >= deadline)
			return false;
		g_system->delayMillis(1);
	}
	return false;
}

} // End of namespace Sci
