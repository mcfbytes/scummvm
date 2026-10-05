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

#ifndef SCI_ENGINE_TSN_ENVELOPE_WRITER_H
#define SCI_ENGINE_TSN_ENVELOPE_WRITER_H

#include "common/array.h"
#include "common/str.h"

#include "sci/engine/tsn_envelope.h"

namespace Sci {

// Builds one envelope as it travels on a TCP stream, length prefix included.
// A field that the transport cannot carry makes the whole envelope invalid.
class TsnEnvelopeWriter {
public:
	TsnEnvelopeWriter(TsnEnvelopeKind kind, uint16 tag);

	static TsnEnvelopeWriter makeHello(const Common::String &clientName);
	static TsnEnvelopeWriter makeCall(uint16 tag, TsnExport exportId);

	void writeByte(byte value);
	void writeUint16(uint16 value);
	void writeBool(bool value);
	void writeStr8(const Common::String &text, uint maxSize = 255);
	void writeBytes16(const Common::Array<byte> &bytes);

	bool isValid() const { return _valid; }
	uint16 tag() const { return _tag; }
	TsnExport exportId() const { return _exportId; }
	Common::Array<byte> frame() const;

private:
	uint16 _tag;
	TsnExport _exportId;
	bool _valid;
	Common::Array<byte> _envelope;
};

} // End of namespace Sci

#endif // SCI_ENGINE_TSN_ENVELOPE_WRITER_H
