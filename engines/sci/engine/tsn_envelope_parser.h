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

#ifndef SCI_ENGINE_TSN_ENVELOPE_PARSER_H
#define SCI_ENGINE_TSN_ENVELOPE_PARSER_H

#include "common/array.h"
#include "common/str.h"

#include "sci/engine/tsn_envelope.h"

namespace Sci {

// Reads the fields of one envelope, without its length prefix. A read past the
// end or a value out of range marks the envelope malformed and reads as zero.
class TsnEnvelopeParser {
public:
	explicit TsnEnvelopeParser(const Common::Array<byte> &envelope);

	byte kind() const { return _kind; }
	uint16 tag() const { return _tag; }

	byte readByte();
	uint16 readUint16();
	bool readBool();
	Common::Array<byte> readBytes16();
	Common::String readStr8();
	void require(bool condition);

	bool isComplete() const { return !_malformed && _position == _envelope.size(); }

private:
	Common::Array<byte> readRun(uint size);

	Common::Array<byte> _envelope;
	uint _position;
	bool _malformed;
	byte _kind;
	uint16 _tag;
};

} // End of namespace Sci

#endif // SCI_ENGINE_TSN_ENVELOPE_PARSER_H
