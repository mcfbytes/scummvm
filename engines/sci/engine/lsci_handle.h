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

#ifndef SCI_ENGINE_LSCI_HANDLE_H
#define SCI_ENGINE_LSCI_HANDLE_H

#include "sci/engine/vm_types.h"

namespace Sci {

struct EngineState;

// LSCI names objects by 16-bit handles, and the host echoes the handles that the
// scripts send it; these map ScummVM references to such handles and back.
uint16 getLsciHandle(EngineState *s, reg_t reference);
reg_t resolveLsciHandle(EngineState *s, reg_t value);

} // End of namespace Sci

#endif // SCI_ENGINE_LSCI_HANDLE_H
