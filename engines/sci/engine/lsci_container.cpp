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

#include "sci/engine/lsci_container.h"

namespace Sci {

enum {
	kItemHeaderSize = 3 ///< tag byte and payload length word
};

bool LsciContainer::Item::isReference(uint32 payloadOffset) const {
	for (uint i = 0; i < fixups.size(); ++i) {
		if (fixups[i] == payloadOffset)
			return true;
	}
	return false;
}

static bool hasFixupList(byte tag) {
	return tag != kLsciItemString && tag != kLsciItemSaidSpec && tag != kLsciItemSynonymTable;
}

static bool checkTag(byte tag, uint index, Common::String &error) {
	switch (tag) {
	case kLsciItemObject:
	case kLsciItemClass:
	case kLsciItemCode:
	case kLsciItemVariables:
	case kLsciItemString:
	case kLsciItemDispatchTable:
	case kLsciItemPropertyDictionary:
		return true;
	case kLsciItemSaidSpec:
	case kLsciItemSynonymTable:
		error = Common::String::format("item %u: unsupported parser item (tag %u)", index, tag);
		return false;
	default:
		error = Common::String::format("item %u: unknown tag %u", index, tag);
		return false;
	}
}

static bool parseFixups(const SciSpan<const byte> &data, uint32 &position, uint index, LsciContainer::Item &item, Common::String &error) {
	if (position + 2 > data.size()) {
		error = Common::String::format("item %u: fixup count past the end", index);
		return false;
	}
	const uint16 count = data.getUint16LEAt(position);
	position += 2;
	if (position + count * 2 > data.size()) {
		error = Common::String::format("item %u: %u fixups run past the end", index, count);
		return false;
	}
	for (uint16 i = 0; i < count; ++i, position += 2) {
		const uint16 offset = data.getUint16LEAt(position);
		if (offset + 2u > item.payload.size()) {
			error = Common::String::format("item %u: fixup outside its %u-byte payload", index, item.payload.size());
			return false;
		}
		item.fixups.push_back(offset);
	}
	return true;
}

static bool parseItem(const SciSpan<const byte> &data, uint32 &position, uint index, LsciContainer::Item &item, Common::String &error) {
	if (position + kItemHeaderSize > data.size()) {
		error = Common::String::format("item %u: header past the end", index);
		return false;
	}
	const byte tag = data.getUint8At(position);
	const uint16 length = data.getUint16LEAt(position + 1);
	position += kItemHeaderSize;
	if (!checkTag(tag, index, error))
		return false;
	if (position + length > data.size()) {
		error = Common::String::format("item %u: payload of %u bytes runs past the end", index, length);
		return false;
	}
	item.tag = (LsciItemTag)tag;
	item.payload = data.subspan(position, length);
	position += length;
	return !hasFixupList(tag) || parseFixups(data, position, index, item, error);
}

static bool checkObjectLayout(const LsciContainer &container, uint index, Common::String &error) {
	const LsciContainer::Item &item = container.items[index];
	const uint32 length = item.payload.size();
	if (length < (kLsciSlotSize + 1) * 2 || item.getPropertyCount() <= kLsciSlotName ||
		item.getPropertyCount() * 2u + 2 > length ||
		length != item.getPropertyCount() * 2u + 2 + item.getMethodCount() * 4u) {
		error = Common::String::format("item %u: object layout does not match its %u bytes", index, length);
		return false;
	}
	if (item.tag != kLsciItemClass)
		return true;
	if (index + 1 >= container.items.size() || container.items[index + 1].tag != kLsciItemPropertyDictionary ||
		container.items[index + 1].payload.size() < item.getPropertyCount() * 2u) {
		error = Common::String::format("item %u: class without its property dictionary", index);
		return false;
	}
	return true;
}

static bool checkItem(const LsciContainer &container, uint index, Common::String &error) {
	const LsciContainer::Item &item = container.items[index];
	for (uint i = 0; i < item.fixups.size(); ++i) {
		if (item.getWord(item.fixups[i]) >= container.items.size()) {
			error = Common::String::format("item %u: reference to missing item %u", index, item.getWord(item.fixups[i]));
			return false;
		}
	}

	const uint32 length = item.payload.size();
	switch (item.tag) {
	case kLsciItemObject:
	case kLsciItemClass:
		return checkObjectLayout(container, index, error);
	case kLsciItemString:
		if (length > kLsciStringTextOffset && item.payload.getUint8At(length - 1) == 0)
			return true;
		error = Common::String::format("item %u: string without its header or terminator", index);
		return false;
	case kLsciItemDispatchTable:
		if (length >= 2 && length == (item.getWord(0) + 2u) * 2)
			return true;
		error = Common::String::format("item %u: dispatch table does not match its %u bytes", index, length);
		return false;
	default:
		return true;
	}
}

bool parseLsciContainer(const SciSpan<const byte> &data, LsciContainer &container, Common::String &error) {
	container.items.clear();
	if (data.size() < 2) {
		error = "shorter than the item count";
		return false;
	}

	const uint16 count = data.getUint16LEAt(0);
	uint32 position = 2;
	for (uint index = 0; index < count; ++index) {
		LsciContainer::Item item;
		if (!parseItem(data, position, index, item, error))
			return false;
		container.items.push_back(item);
	}
	if (position != data.size()) {
		error = Common::String::format("%u bytes after the last item", data.size() - position);
		return false;
	}

	for (uint index = 0; index < count; ++index) {
		if (!checkItem(container, index, error))
			return false;
	}
	return true;
}

} // End of namespace Sci
