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

#ifndef SCI_ENGINE_TSN_ENVELOPE_H
#define SCI_ENGINE_TSN_ENVELOPE_H

#include "sci/engine/tsn_executive.h"

namespace Sci {

// Vocabulary of the INT 14h transport to innkeeperd (docs/protocol/int14h-transport.md)
enum TsnEnvelopeKind {
	kTsnEnvelopeHello = 0x01,
	kTsnEnvelopeWelcome = 0x02,
	kTsnEnvelopeCall = 0x10,
	kTsnEnvelopeReply = 0x11
};

enum {
	kTsnTransportVersion = 1,
	kTsnLengthPrefixSize = 4,
	kTsnMaxEnvelopeSize = 0x10100,
	kTsnMaxDialStringSize = 127
};

} // End of namespace Sci

#endif // SCI_ENGINE_TSN_ENVELOPE_H
