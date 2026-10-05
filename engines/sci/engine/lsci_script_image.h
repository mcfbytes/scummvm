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

#ifndef SCI_ENGINE_LSCI_SCRIPT_IMAGE_H
#define SCI_ENGINE_LSCI_SCRIPT_IMAGE_H

#include "common/array.h"
#include "common/str.h"

#include "sci/engine/lsci_container.h"

namespace Sci {

// Writes an LSCI container as an SCI0 script image: one block per item in item order,
// with every item reference resolved to an image offset
class LsciScriptImageWriter {
public:
	// A branch that leaves its Code item; the original compiler emits a few of these
	struct StrayBranch {
		uint16 item;
		uint16 offset;
		int32 target;
	};

	LsciScriptImageWriter() : _container(nullptr), _image(nullptr) {}

	bool write(const LsciContainer &container, Common::Array<byte> &image, Common::String &error);

	const Common::Array<StrayBranch> &getStrayBranches() const { return _strayBranches; }

private:
	bool layout(Common::String &error);
	bool checkReferences(Common::String &error) const;
	bool writeBlock(uint index, Common::String &error);
	void writeObject(uint index, uint32 bodyOffset);
	bool writeCode(uint index, uint32 bodyOffset, Common::String &error);
	bool rewriteInstruction(uint index, uint32 bodyOffset, uint32 pc, uint32 size, uint &references, Common::String &error);
	void writeVariables(uint index, uint32 bodyOffset);
	void writeExports(uint index, uint32 bodyOffset);
	void writeRelocations();

	uint16 getTarget(const LsciContainer::Item &item, uint32 payloadOffset) const;
	uint16 resolveWord(const LsciContainer::Item &item, uint32 payloadOffset, uint32 imageOffset);
	void copyBytes(uint32 imageOffset, const SciSpan<const byte> &bytes);
	void setWord(uint32 imageOffset, uint16 value);

	const LsciContainer *_container; ///< borrowed during write()
	Common::Array<byte> *_image;     ///< borrowed during write()
	Common::Array<uint32> _blockOffsets;
	Common::Array<uint32> _itemOffsets;
	Common::Array<uint16> _relocations;
	Common::Array<StrayBranch> _strayBranches;
};

} // End of namespace Sci

#endif // SCI_ENGINE_LSCI_SCRIPT_IMAGE_H
