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

#ifndef SCI_ENGINE_LSCI_KERNEL_H
#define SCI_ENGINE_LSCI_KERNEL_H

#include "sci/engine/vm_types.h"

namespace Sci {

struct EngineState;

#ifdef ENABLE_SCI32
reg_t kLsciGetEvent(EngineState *s, int argc, reg_t *argv);

reg_t kTsn(EngineState *s, int argc, reg_t *argv);
reg_t kTsnGetStatus(EngineState *s, int argc, reg_t *argv);
reg_t kTsnGetSharedData(EngineState *s, int argc, reg_t *argv);
reg_t kTsnSetSharedData(EngineState *s, int argc, reg_t *argv);
reg_t kTsnConnect(EngineState *s, int argc, reg_t *argv);
reg_t kTsnSend(EngineState *s, int argc, reg_t *argv);
reg_t kTsnReserved(EngineState *s, int argc, reg_t *argv);
reg_t kTsnDisconnect(EngineState *s, int argc, reg_t *argv);
reg_t kTsnPoll(EngineState *s, int argc, reg_t *argv);
reg_t kTsnSetNextProgram(EngineState *s, int argc, reg_t *argv);
reg_t kTsnGetPreviousProgram(EngineState *s, int argc, reg_t *argv);
reg_t kTsnIsTransmitIdle(EngineState *s, int argc, reg_t *argv);
reg_t kTsnSwitchHost(EngineState *s, int argc, reg_t *argv);
reg_t kTsnFlush(EngineState *s, int argc, reg_t *argv);
reg_t kTsnGetLineRate(EngineState *s, int argc, reg_t *argv);
#endif

} // End of namespace Sci

#endif // SCI_ENGINE_LSCI_KERNEL_H
