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

#ifndef SCI_ENGINE_TSN_SOCKET_TRANSPORT_H
#define SCI_ENGINE_TSN_SOCKET_TRANSPORT_H

#include "common/ptr.h"
#include "common/str.h"

#include "sci/engine/tsn_transport.h"

namespace Networking {
class Socket;
}

namespace Sci {

// A TCP connection through ScummVM's basic networking
class TsnSocketTransport : public TsnTransport {
public:
	TsnSocketTransport(const Common::String &host, int port);
	~TsnSocketTransport() override;

	bool open() override;
	void close() override;
	bool send(const Common::Array<byte> &bytes) override;
	bool receive(Common::Array<byte> &bytes) override;

private:
	Common::String _url;
	Common::ScopedPtr<Networking::Socket> _socket;
};

} // End of namespace Sci

#endif // SCI_ENGINE_TSN_SOCKET_TRANSPORT_H
