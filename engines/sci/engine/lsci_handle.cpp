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

#include "sci/engine/lsci_handle.h"
#include "sci/engine/state.h"

namespace Sci {

// Handles start high so that small script numbers never name an object
enum {
	kLsciFirstHandle = 0x8000
};

uint16 getLsciHandle(EngineState *s, reg_t reference) {
	Common::Array<reg_t> &handles = s->_lsciHandles;
	for (uint i = 0; i < handles.size(); ++i) {
		if (handles[i] == reference)
			return kLsciFirstHandle + i;
	}
	handles.push_back(reference);
	return kLsciFirstHandle + handles.size() - 1;
}

reg_t resolveLsciHandle(EngineState *s, reg_t value) {
	const uint index = value.getOffset() - kLsciFirstHandle;
	if (value.getSegment() || value.getOffset() < kLsciFirstHandle || index >= s->_lsciHandles.size())
		return value;
	return s->_lsciHandles[index];
}

} // End of namespace Sci
