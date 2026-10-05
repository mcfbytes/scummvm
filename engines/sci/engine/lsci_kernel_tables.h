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

#ifndef SCI_ENGINE_LSCI_KERNEL_TABLES_H
#define SCI_ENGINE_LSCI_KERNEL_TABLES_H

#include "sci/engine/kernel_tables.h"
#include "sci/engine/lsci_kernel.h"

namespace Sci {

// LSCI's built-in kernel table in dispatch order, the same in every known build.
static const char *const s_lsciKernelNames[] = {
	"IsObject",         // 0x00
	"ObjectFree",       // 0x01
	"ObjectNew",        // 0x02
	"ObjectRespondsTo", // 0x03
	"Array",            // 0x04
	"Block",            // 0x05
	"List",             // 0x06
	"ModuleDispose",    // 0x07
	"ModuleID",         // 0x08
	"Resource",         // 0x09
	"Seq",              // 0x0a
	"String",           // 0x0b
	"GetFarText",       // 0x0c
	"CoordPri",         // 0x0d
	"DrawPic",          // 0x0e
	"PicState",         // 0x0f
	"ShakeScreen",      // 0x10
	"Show",             // 0x11
	"AddToPic",         // 0x12
	"Animate",          // 0x13
	"CelHigh",          // 0x14
	"CelRect",          // 0x15
	"CelWide",          // 0x16
	"DrawCel",          // 0x17
	"DrawScaledCel",    // 0x18
	"NumCels",          // 0x19
	"NumLoops",         // 0x1a
	"ScaledCelRect",    // 0x1b
	"DrawControl",      // 0x1c
	"EditControl",      // 0x1d
	"HiliteControl",    // 0x1e
	"GetPort",          // 0x1f
	"SetPort",          // 0x20
	"WindowDispose",    // 0x21
	"WindowNew",        // 0x22
	"Display",          // 0x23
	"TextSize",         // 0x24
	"GetEvent",         // 0x25
	"GlobalToLocal",    // 0x26
	"LocalToGlobal",    // 0x27
	"MapKeyToDir",      // 0x28
	"Menu",             // 0x29
	"HaveMouse",        // 0x2a
	"SetCursor",        // 0x2b
	"Memory",           // 0x2c
	"CheckSaveGame",    // 0x2d
	"GameIsRestarting", // 0x2e
	"GetSaveDir",       // 0x2f
	"GetSaveFiles",     // 0x30
	"Restart",          // 0x31
	"Restore",          // 0x32
	"Save",             // 0x33
	"Sound",            // 0x34
	"GetTime",          // 0x35
	"SetTimerFreq",     // 0x36
	"Wait",             // 0x37
	"Alert",            // 0x38
	"CanBeHere",        // 0x39
	"OnControl",        // 0x3a
	"FileSystem",       // 0x3b
	"DeviceInfo",       // 0x3c
	"Abs",              // 0x3d
	"CosDiv",           // 0x3e
	"CosMult",          // 0x3f
	"GetAngle",         // 0x40
	"GetDistance",      // 0x41
	"Random",           // 0x42
	"SinDiv",           // 0x43
	"SinMult",          // 0x44
	"Sqrt",             // 0x45
	"Long",             // 0x46
	"Graph",            // 0x47
	"Debug",            // 0x48
	"Profiler",         // 0x49
	"SetDebug",         // 0x4a
	"ShowFree",         // 0x4b
	"StackUsage",       // 0x4c
	"Joystick",         // 0x4d
	"BaseRectSet",      // 0x4e
	"ConfigStr",        // 0x4f
	"Encrypt",          // 0x50
	"Decrypt",          // 0x51
	"ColorUp",          // 0x52
	"ColorDown",        // 0x53
	"TSN",              // 0x54
	"ObjPropOffset",    // 0x55
	"ObjOffsetProp",    // 0x56
	"SID",              // 0x57
	"InvokeMethod"      // 0x58
};

// Kernels that GRAPH256.DLL and NLNULL.DLL append at start-up, numbered on the
// assumption that they load in this order; scripts call exactly these numbers.
static const char *const s_lsciDllKernelNamesAssumed[] = {
	"Palette",          // 0x59
	"Said",             // 0x5a
	"Parse",            // 0x5b
	"SetSynonyms"       // 0x5c
};

#ifdef ENABLE_SCI32
// The sub-op number is the TSN executive's export number; four exports are not for scripts
static const SciKernelMapSubEntry kTsn_subops[] = {
	{ SIG_SCIALL,  0, MAP_CALL(TsnGetStatus),          "",          nullptr },
	{ SIG_SCIALL,  1, MAP_CALL(TsnGetSharedData),      "",          nullptr },
	{ SIG_SCIALL,  2, MAP_CALL(TsnSetSharedData),      "[r0]i",     nullptr },
	{ SIG_SCIALL,  3, MAP_CALL(TsnConnect),            "[ro]",      nullptr },
	{ SIG_SCIALL,  4, MAP_CALL(TsnSend),               "[ro](.*)",  nullptr },
	{ SIG_SCIALL,  5, MAP_CALL(TsnReserved),           "(.*)",      nullptr },
	{ SIG_SCIALL,  6, MAP_CALL(TsnReserved),           "(.*)",      nullptr },
	{ SIG_SCIALL,  7, MAP_CALL(TsnDisconnect),         "",          nullptr },
	{ SIG_SCIALL,  8, MAP_CALL(TsnPoll),               "",          nullptr },
	{ SIG_SCIALL,  9, MAP_CALL(TsnSetNextProgram),     "[ro0]",     nullptr },
	{ SIG_SCIALL, 10, MAP_CALL(TsnReserved),           "(.*)",      nullptr },
	{ SIG_SCIALL, 11, MAP_CALL(TsnGetPreviousProgram), "",          nullptr },
	{ SIG_SCIALL, 12, MAP_CALL(TsnIsTransmitIdle),     "",          nullptr },
	{ SIG_SCIALL, 13, MAP_CALL(TsnSwitchHost),         "[ro]",      nullptr },
	{ SIG_SCIALL, 14, MAP_CALL(TsnFlush),              "",          nullptr },
	{ SIG_SCIALL, 15, MAP_CALL(TsnReserved),           "(.*)",      nullptr },
	{ SIG_SCIALL, 16, MAP_CALL(TsnGetLineRate),        "",          nullptr },
	SCI_SUBOPENTRY_TERMINATOR
};
#endif

// LSCI kernels by name. Unlisted names fall back to kStub; several clash with SCI
// kernels of another meaning. LSCI's own kernels need SCI32's arrays.
static const SciKernelMapEntry s_lsciKernelMap[] = {
	{ MAP_CALL(IsObject),                SIG_EVERYWHERE, ".",              nullptr, nullptr },
	{ "ObjectRespondsTo", kRespondsTo,   SIG_EVERYWHERE, ".i",             nullptr, nullptr },
	{ "ModuleID", kScriptID,             SIG_EVERYWHERE, "[io](i)",        nullptr, nullptr },
	{ MAP_CALL(GetFarText),              SIG_EVERYWHERE, "ii[r0]",         nullptr, nullptr },
	{ MAP_CALL(MapKeyToDir),             SIG_EVERYWHERE, "o",              nullptr, nullptr },
	{ MAP_CALL(HaveMouse),               SIG_EVERYWHERE, "",               nullptr, nullptr },
	{ MAP_EMPTY(CheckSaveGame),          SIG_EVERYWHERE, "(.*)",           nullptr, nullptr },
	{ MAP_CALL(GameIsRestarting),        SIG_EVERYWHERE, "(i)",            nullptr, nullptr },
	{ MAP_EMPTY(GetSaveDir),             SIG_EVERYWHERE, "(.*)",           nullptr, nullptr },
	{ MAP_EMPTY(GetSaveFiles),           SIG_EVERYWHERE, "(.*)",           nullptr, nullptr },
	{ MAP_CALL(GetTime),                 SIG_EVERYWHERE, "(i)",            nullptr, nullptr },
	{ MAP_EMPTY(DeviceInfo),             SIG_EVERYWHERE, "(.*)",           nullptr, nullptr },
	{ MAP_CALL(Abs),                     SIG_EVERYWHERE, "i",              nullptr, nullptr },
	{ MAP_CALL(CosDiv),                  SIG_EVERYWHERE, "ii",             nullptr, nullptr },
	{ "CosMult", kTimesCos,              SIG_EVERYWHERE, "ii",             nullptr, nullptr },
	{ MAP_CALL(GetAngle),                SIG_EVERYWHERE, "iiii",           nullptr, nullptr },
	{ MAP_CALL(GetDistance),             SIG_EVERYWHERE, "ii(i)(i)(i)(i)", nullptr, nullptr },
	{ MAP_CALL(Random),                  SIG_EVERYWHERE, "(i)(i)",         nullptr, nullptr },
	{ MAP_CALL(SinDiv),                  SIG_EVERYWHERE, "ii",             nullptr, nullptr },
	{ "SinMult", kTimesSin,              SIG_EVERYWHERE, "ii",             nullptr, nullptr },
	{ "Debug", kStubNull,                SIG_EVERYWHERE, "(.*)",           nullptr, nullptr },
	{ MAP_EMPTY(Profiler),               SIG_EVERYWHERE, "(.*)",           nullptr, nullptr },
	{ MAP_EMPTY(SetDebug),               SIG_EVERYWHERE, "(.*)",           nullptr, nullptr },
	{ MAP_EMPTY(ShowFree),               SIG_EVERYWHERE, "(.*)",           nullptr, nullptr },
	{ MAP_CALL(Joystick),                SIG_EVERYWHERE, "i(.*)",          nullptr, nullptr },
	{ MAP_CALL(ScummVMSleep),            SIG_EVERYWHERE, "i",              nullptr, nullptr },
#ifdef ENABLE_SCI32
	{ "GetEvent", kLsciGetEvent,         SIG_EVERYWHERE, "io",             nullptr, nullptr },
	{ "TSN", kTsn,                       SIG_EVERYWHERE, "i(.*)",          kTsn_subops, nullptr },
#else
	{ MAP_CALL(GetEvent),                SIG_EVERYWHERE, "io",             nullptr, nullptr },
#endif
	{ nullptr, nullptr,                  SIG_EVERYWHERE, nullptr,          nullptr, nullptr }
};

} // End of namespace Sci

#endif // SCI_ENGINE_LSCI_KERNEL_TABLES_H
