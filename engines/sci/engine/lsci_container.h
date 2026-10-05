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

#ifndef SCI_ENGINE_LSCI_CONTAINER_H
#define SCI_ENGINE_LSCI_CONTAINER_H

#include "common/array.h"
#include "common/str.h"

#include "sci/util.h"

namespace Sci {

// Item tags are the memory-block types of the LSCI interpreter
enum LsciItemTag {
	kLsciItemObject = 2,
	kLsciItemClass = 3,
	kLsciItemCode = 4,
	kLsciItemVariables = 6,
	kLsciItemString = 7,
	kLsciItemSaidSpec = 8,
	kLsciItemDispatchTable = 9,
	kLsciItemSynonymTable = 10,
	kLsciItemPropertyDictionary = 11
};

// Leading property slots of every LSCI object and class
enum LsciObjectSlot {
	kLsciSlotEnv = 0,
	kLsciSlotSize = 1,
	kLsciSlotDict = 2,
	kLsciSlotSuper = 3,
	kLsciSlotInfo = 4,
	kLsciSlotName = 5
};

enum {
	kLsciStringTextOffset = 4 ///< a String item's text follows its kind and size words
};

// An LSCI script or type-31 module: numbered items, in which every word listed as
// a fixup holds the number of another item
struct LsciContainer {
	struct Item {
		LsciItemTag tag;
		SciSpan<const byte> payload;  ///< borrowed from the resource
		Common::Array<uint16> fixups; ///< payload offsets of the words that hold item numbers

		uint16 getWord(uint32 payloadOffset) const { return payload.getUint16LEAt(payloadOffset); }
		bool isReference(uint32 payloadOffset) const;
		uint16 getPropertyCount() const { return getWord(kLsciSlotSize * 2); }
		uint16 getMethodCount() const { return getWord(getPropertyCount() * 2); }
	};

	Common::Array<Item> items;
};

// Returns false with a reason when the data is not a supported item container
bool parseLsciContainer(const SciSpan<const byte> &data, LsciContainer &container, Common::String &error);

} // End of namespace Sci

#endif // SCI_ENGINE_LSCI_CONTAINER_H
