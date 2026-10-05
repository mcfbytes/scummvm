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

#ifndef SCI_ENGINE_TSN_TRANSPORT_H
#define SCI_ENGINE_TSN_TRANSPORT_H

#include "common/array.h"

namespace Sci {

// A byte stream to the host: a TCP socket, or a loopback in the tests
class TsnTransport {
public:
	virtual ~TsnTransport() {}

	virtual bool open() = 0;
	virtual void close() = 0;
	virtual bool send(const Common::Array<byte> &bytes) = 0;
	// Appends at least one byte; false when the stream ended or nothing came in time
	virtual bool receive(Common::Array<byte> &bytes) = 0;
};

} // End of namespace Sci

#endif // SCI_ENGINE_TSN_TRANSPORT_H
