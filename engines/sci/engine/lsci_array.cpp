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

#include "sci/engine/kernel.h"
#include "sci/engine/lsci_array.h"
#include "sci/engine/seg_manager.h"
#include "sci/engine/selector.h"

namespace Sci {

Common::String getLsciString(SegManager *segMan, reg_t stringOrObject) {
	if (segMan->isObject(stringOrObject))
		return segMan->getString(readSelector(segMan, stringOrObject, SELECTOR(data)));
	return segMan->getString(stringOrObject);
}

uint getLsciElementSize(SegManager *segMan, reg_t array) {
	if (!segMan->isArray(array))
		return 1;
	const SciArrayType type = segMan->lookupArray(array)->getType();
	return (type == kArrayTypeByte || type == kArrayTypeString) ? 1 : 2;
}

Common::Array<byte> getLsciArrayBytes(SegManager *segMan, reg_t array, uint byteCount) {
	Common::Array<byte> bytes;
	if (!segMan->isArray(array)) {
		const SegmentRef literal = segMan->dereference(array);
		if (!literal.isRaw || !literal.raw) {
			warning("LSCI: %04x:%04x is neither an array nor a string literal", PRINT_REG(array));
			return bytes;
		}
		bytes.resize(MIN<uint>(byteCount, literal.maxSize));
		memcpy(bytes.data(), literal.raw, bytes.size());
		return bytes;
	}

	SciArray &source = *segMan->lookupArray(array);
	byteCount = MIN<uint>(byteCount, source.byteSize());
	bytes.resize(byteCount);
	if (getLsciElementSize(segMan, array) == 1) {
		memcpy(bytes.data(), source.getRawData(), byteCount);
		return bytes;
	}

	for (uint i = 0; i < byteCount; ++i) {
		const uint16 word = source.IDAt(i / 2).getOffset();
		bytes[i] = (i & 1) ? (word >> 8) : (word & 0xFF);
	}
	return bytes;
}

void setLsciArrayBytes(SegManager *segMan, reg_t array, const Common::Array<byte> &bytes) {
	SciArray &target = *segMan->lookupArray(array);
	if (getLsciElementSize(segMan, array) == 1) {
		target.resize(bytes.size());
		if (!bytes.empty())
			memcpy(target.getRawData(), bytes.data(), bytes.size());
		return;
	}

	target.resize((bytes.size() + 1) / 2);
	for (uint i = 0; i < bytes.size(); i += 2) {
		const uint16 high = i + 1 < bytes.size() ? bytes[i + 1] : 0;
		target.IDAt(i / 2) = make_reg(0, bytes[i] | (high << 8));
	}
}

reg_t makeLsciByteArray(SegManager *segMan, const Common::Array<byte> &bytes) {
	reg_t handle;
	SciArray *array = segMan->allocateArray(kArrayTypeByte, bytes.size(), &handle);
	if (!bytes.empty())
		memcpy(array->getRawData(), bytes.data(), bytes.size());
	return handle;
}

reg_t makeLsciString(SegManager *segMan, const Common::String &text) {
	reg_t handle;
	SciArray *array = segMan->allocateArray(kArrayTypeString, text.size() + 1, &handle);
	array->fromString(text);
	return handle;
}

} // End of namespace Sci
