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

#include "sci/engine/tsn_envelope_parser.h"

namespace Sci {

TsnEnvelopeParser::TsnEnvelopeParser(const Common::Array<byte> &envelope) :
	_envelope(envelope),
	_position(0),
	_malformed(false),
	_kind(0),
	_tag(0) {
	_kind = readByte();
	_tag = readUint16();
}

byte TsnEnvelopeParser::readByte() {
	if (_position >= _envelope.size()) {
		_malformed = true;
		return 0;
	}
	return _envelope[_position++];
}

uint16 TsnEnvelopeParser::readUint16() {
	const byte low = readByte();
	return low | (readByte() << 8);
}

bool TsnEnvelopeParser::readBool() {
	const byte value = readByte();
	require(value <= 1);
	return value == 1;
}

Common::Array<byte> TsnEnvelopeParser::readBytes16() {
	return readRun(readUint16());
}

Common::String TsnEnvelopeParser::readStr8() {
	const Common::Array<byte> text = readRun(readByte());
	return text.empty() ? Common::String() : Common::String((const char *)text.data(), text.size());
}

void TsnEnvelopeParser::require(bool condition) {
	if (!condition)
		_malformed = true;
}

Common::Array<byte> TsnEnvelopeParser::readRun(uint size) {
	if (size > _envelope.size() - _position) {
		_malformed = true;
		return Common::Array<byte>();
	}
	const Common::Array<byte> run(_envelope.data() + _position, size);
	_position += size;
	return run;
}

} // End of namespace Sci
