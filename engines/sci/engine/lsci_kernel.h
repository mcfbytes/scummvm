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
reg_t kLsciResource(EngineState *s, int argc, reg_t *argv);
reg_t kLsciGetFarText(EngineState *s, int argc, reg_t *argv);
reg_t kObjectNew(EngineState *s, int argc, reg_t *argv);
reg_t kLsciConfigStr(EngineState *s, int argc, reg_t *argv);
reg_t kLsciArrayNew(EngineState *s, int argc, reg_t *argv);
reg_t kLsciArrayAt(EngineState *s, int argc, reg_t *argv);
reg_t kLsciArrayAtPut(EngineState *s, int argc, reg_t *argv);
reg_t kLsciArrayCompare(EngineState *s, int argc, reg_t *argv);
reg_t kLsciWait(EngineState *s, int argc, reg_t *argv);
reg_t kLsciBlock(EngineState *s, int argc, reg_t *argv);
reg_t kLsciModuleDispose(EngineState *s, int argc, reg_t *argv);
reg_t kLsciMemory(EngineState *s, int argc, reg_t *argv);
reg_t kLsciStringGetToken(EngineState *s, int argc, reg_t *argv);
reg_t kLsciLong(EngineState *s, int argc, reg_t *argv);
reg_t kLsciSid(EngineState *s, int argc, reg_t *argv);
reg_t kLsciIsObject(EngineState *s, int argc, reg_t *argv);
reg_t kLsciObjPropOffset(EngineState *s, int argc, reg_t *argv);
reg_t kLsciObjOffsetProp(EngineState *s, int argc, reg_t *argv);

reg_t kLsciSetCursor(EngineState *s, int argc, reg_t *argv);
reg_t kLsciAnimate(EngineState *s, int argc, reg_t *argv);
reg_t kLsciGraphSaveBits(EngineState *s, int argc, reg_t *argv);
reg_t kLsciGraphFillRect(EngineState *s, int argc, reg_t *argv);
reg_t kLsciGraphShowBits(EngineState *s, int argc, reg_t *argv);
reg_t kLsciGraphRedrawBox(EngineState *s, int argc, reg_t *argv);
reg_t kLsciTextSize(EngineState *s, int argc, reg_t *argv);
reg_t kLsciDisplay(EngineState *s, int argc, reg_t *argv);
reg_t kLsciCelRect(EngineState *s, int argc, reg_t *argv);
reg_t kLsciDrawCel(EngineState *s, int argc, reg_t *argv);
reg_t kLsciWindowNew(EngineState *s, int argc, reg_t *argv);
reg_t kLsciSetPort(EngineState *s, int argc, reg_t *argv);
reg_t kLsciDrawControl(EngineState *s, int argc, reg_t *argv);
reg_t kLsciHiliteControl(EngineState *s, int argc, reg_t *argv);
reg_t kLsciEditControl(EngineState *s, int argc, reg_t *argv);
reg_t kLsciCanBeHere(EngineState *s, int argc, reg_t *argv);
reg_t kLsciOnControl(EngineState *s, int argc, reg_t *argv);
reg_t kLsciNumLoops(EngineState *s, int argc, reg_t *argv);
reg_t kLsciNumCels(EngineState *s, int argc, reg_t *argv);
reg_t kLsciBaseRectSet(EngineState *s, int argc, reg_t *argv);

reg_t kLsciList(EngineState *s, int argc, reg_t *argv);
reg_t kLsciListNew(EngineState *s, int argc, reg_t *argv);
reg_t kLsciListDispose(EngineState *s, int argc, reg_t *argv);
reg_t kLsciListCount(EngineState *s, int argc, reg_t *argv);
reg_t kLsciListAt(EngineState *s, int argc, reg_t *argv);
reg_t kLsciListDelete(EngineState *s, int argc, reg_t *argv);
reg_t kLsciListContains(EngineState *s, int argc, reg_t *argv);
reg_t kLsciListAddToFront(EngineState *s, int argc, reg_t *argv);
reg_t kLsciListAddToEnd(EngineState *s, int argc, reg_t *argv);
reg_t kLsciListAddAfter(EngineState *s, int argc, reg_t *argv);
reg_t kLsciListAddBefore(EngineState *s, int argc, reg_t *argv);
reg_t kLsciListMoveToFront(EngineState *s, int argc, reg_t *argv);
reg_t kLsciListMoveToEnd(EngineState *s, int argc, reg_t *argv);
reg_t kLsciListIndexOf(EngineState *s, int argc, reg_t *argv);
reg_t kLsciListRelease(EngineState *s, int argc, reg_t *argv);

reg_t kLsciSeq(EngineState *s, int argc, reg_t *argv);
reg_t kLsciSeqNew(EngineState *s, int argc, reg_t *argv);
reg_t kLsciSeqDispose(EngineState *s, int argc, reg_t *argv);
reg_t kLsciSeqBegin(EngineState *s, int argc, reg_t *argv);
reg_t kLsciSeqEnd(EngineState *s, int argc, reg_t *argv);
reg_t kLsciSeqNext(EngineState *s, int argc, reg_t *argv);
reg_t kLsciSeqPrev(EngineState *s, int argc, reg_t *argv);
reg_t kLsciSeqOver(EngineState *s, int argc, reg_t *argv);

reg_t kLsciFileOpen(EngineState *s, int argc, reg_t *argv);
reg_t kLsciFileWriteString(EngineState *s, int argc, reg_t *argv);
reg_t kLsciFileReadString(EngineState *s, int argc, reg_t *argv);
reg_t kLsciFileGetByte(EngineState *s, int argc, reg_t *argv);
reg_t kLsciFileSeek(EngineState *s, int argc, reg_t *argv);
reg_t kLsciFileReadToArray(EngineState *s, int argc, reg_t *argv);
reg_t kLsciFileWriteFromArray(EngineState *s, int argc, reg_t *argv);
reg_t kLsciFileSize(EngineState *s, int argc, reg_t *argv);
reg_t kLsciFileSizeHigh(EngineState *s, int argc, reg_t *argv);
reg_t kLsciFileIsDirectory(EngineState *s, int argc, reg_t *argv);

reg_t kLsciGetEvent(EngineState *s, int argc, reg_t *argv);

reg_t kTsn(EngineState *s, int argc, reg_t *argv);
#endif

} // End of namespace Sci

#endif // SCI_ENGINE_LSCI_KERNEL_H
