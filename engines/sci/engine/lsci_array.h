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

#ifndef SCI_ENGINE_LSCI_ARRAY_H
#define SCI_ENGINE_LSCI_ARRAY_H

#include "common/array.h"
#include "common/str.h"

#include "sci/engine/vm_types.h"

namespace Sci {

class SegManager;

// LSCI arrays and strings are SCI32 arrays of the same four element types. These
// helpers give the original's byte view of them (docs/protocol/ktsn.md).

// A string handle or literal, or an object whose data property holds one
Common::String getLsciString(SegManager *segMan, reg_t stringOrObject);

// 1 for byte and string arrays, 2 for the others
uint getLsciElementSize(SegManager *segMan, reg_t array);

// The first byteCount bytes from element 0, two-byte elements little-endian;
// a string literal in a script reads as its bytes
Common::Array<byte> getLsciArrayBytes(SegManager *segMan, reg_t array, uint byteCount);

// Stores bytes from element 0 in the same layout, growing the array to fit them
void setLsciArrayBytes(SegManager *segMan, reg_t array, const Common::Array<byte> &bytes);

reg_t makeLsciByteArray(SegManager *segMan, const Common::Array<byte> &bytes);
reg_t makeLsciString(SegManager *segMan, const Common::String &text);

} // End of namespace Sci

#endif // SCI_ENGINE_LSCI_ARRAY_H
