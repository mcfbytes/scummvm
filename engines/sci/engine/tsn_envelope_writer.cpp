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

#include "common/endian.h"

#include "sci/engine/tsn_envelope_writer.h"

namespace Sci {

static const byte kTsnHelloMagic[] = { 'I', '1', '4', 'H' };

TsnEnvelopeWriter::TsnEnvelopeWriter(TsnEnvelopeKind kind, uint16 tag) :
	_tag(tag),
	_exportId(kTsnExportCount),
	_valid(true) {
	writeByte(kind);
	writeUint16(tag);
}

TsnEnvelopeWriter TsnEnvelopeWriter::makeHello(const Common::String &clientName) {
	TsnEnvelopeWriter hello(kTsnEnvelopeHello, 0);
	for (uint i = 0; i < ARRAYSIZE(kTsnHelloMagic); ++i)
		hello.writeByte(kTsnHelloMagic[i]);
	hello.writeByte(kTsnTransportVersion);
	hello.writeStr8(clientName);
	return hello;
}

TsnEnvelopeWriter TsnEnvelopeWriter::makeCall(uint16 tag, TsnExport exportId) {
	TsnEnvelopeWriter call(kTsnEnvelopeCall, tag);
	call._exportId = exportId;
	call.writeByte(exportId);
	return call;
}

void TsnEnvelopeWriter::writeByte(byte value) {
	_envelope.push_back(value);
}

void TsnEnvelopeWriter::writeUint16(uint16 value) {
	writeByte(value & 0xFF);
	writeByte(value >> 8);
}

void TsnEnvelopeWriter::writeBool(bool value) {
	writeByte(value ? 1 : 0);
}

void TsnEnvelopeWriter::writeStr8(const Common::String &text, uint maxSize) {
	if (text.size() > maxSize)
		_valid = false;
	for (uint i = 0; i < text.size(); ++i) {
		const byte character = text[i];
		if (character < 0x20 || character > 0x7E)
			_valid = false;
	}
	writeByte(text.size() & 0xFF);
	_envelope.push_back(Common::Array<byte>((const byte *)text.c_str(), text.size()));
}

void TsnEnvelopeWriter::writeBytes16(const Common::Array<byte> &bytes) {
	if (bytes.size() > 0xFFFF)
		_valid = false;
	writeUint16(bytes.size() & 0xFFFF);
	_envelope.push_back(bytes);
}

Common::Array<byte> TsnEnvelopeWriter::frame() const {
	Common::Array<byte> framed(kTsnLengthPrefixSize);
	WRITE_LE_UINT32(framed.data(), _envelope.size());
	framed.push_back(_envelope);
	return framed;
}

} // End of namespace Sci
