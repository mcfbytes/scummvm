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

#include "common/config-manager.h"
#include "common/file.h"

#include "sci/sci.h"
#include "sci/engine/kernel.h"
#include "sci/engine/lsci_array.h"
#include "sci/engine/lsci_kernel.h"
#include "sci/engine/selector.h"
#include "sci/engine/state.h"
#include "sci/graphics/paint16.h"
#include "sci/resource/lsci_land.h"

namespace Sci {

enum LsciResourceMode {
	kLsciResourceLoad = 0,
	kLsciResourceUnload = 1,
	kLsciResourceLock = 2
};

enum {
	kLsciConfigLineLimit = 200
};

enum LsciMemoryQuery {
	kLsciMemoryFreeBytesLow = 2,
	kLsciMemoryFreeBytesHigh = 3,
	kLsciMemoryLargestBlockLow = 4,
	kLsciMemoryLargestBlockHigh = 5,
	kLsciMemoryHandleCount = 6,
	kLsciMemoryFreeHandle = 8,
	kLsciMemoryHeapKilobytes = 10
};

enum LsciBlockOp {
	kLsciBlockCopy = 0,
	kLsciBlockFree = 1
};

enum LsciLongOp {
	kLsciLongAdd = 0,
	kLsciLongSubtract = 1,
	kLsciLongMultiply = 2,
	kLsciLongDivide = 3,
	kLsciLongSquareRoot = 4,
	kLsciLongNegate = 5,
	kLsciLongGreaterThan = 6,
	kLsciLongLessThan = 7,
	kLsciLongEquals = 8,
	kLsciLongGreaterOrEqual = 9,
	kLsciLongLessOrEqual = 10,
	kLsciLongAsString = 11,
	kLsciLongFromString = 12
};

enum LsciSidOp {
	kLsciSidInit = 0,
	kLsciSidBind = 1,
	kLsciSidUnbind = 2,
	kLsciSidLookup = 3
};

// LSCITV reports its DOS heap, whose size ScummVM does not have; this one is
// large enough for the rooms' memory checks (class_88) to pass.
static const uint32 kLsciAssumedHeapBytes = 512 * 1024;

static const char *const s_lsciConfigDelimiters = "=; \t";

// LSCI.CFG lookup as LSCITV does it: the first line whose first token equals
// the key, case-sensitively; the value is the rest after the delimiters.
static bool findLsciConfigValue(Common::SeekableReadStream &file, const Common::String &key, Common::String &value) {
	while (!file.eos() && !file.err()) {
		Common::String line = file.readLine();
		if (line.size() > kLsciConfigLineLimit)
			line = Common::String(line.c_str(), kLsciConfigLineLimit);

		const char *text = line.c_str();
		text += strspn(text, s_lsciConfigDelimiters);
		if (!*text || *text == ';')
			continue;

		const size_t keyLength = strcspn(text, s_lsciConfigDelimiters);
		if (key != Common::String(text, keyLength))
			continue;

		text += keyLength;
		value = text + strspn(text, s_lsciConfigDelimiters);
		return true;
	}
	return false;
}

reg_t kLsciConfigStr(EngineState *s, int argc, reg_t *argv) {
	const Common::Path landDirectory = getLsciLandDirectory(ConfMan.get("lsci_land"));
	Common::File file;
	Common::String value;
	if (!file.open(landDirectory.join("lsci.cfg")) ||
		!findLsciConfigValue(file, getLsciString(s->_segMan, argv[0]), value))
		return NULL_REG;
	return makeLsciString(s->_segMan, value);
}

reg_t kObjectNew(EngineState *s, int argc, reg_t *argv) {
	// A copy whose superclass is the source, whether that is a class or an
	// instance; unlike kClone, the species stays that of the source's class.
	const reg_t source = argv[0];
	const reg_t species = s->_segMan->getObject(source)->getSpeciesSelector();
	const reg_t object = kClone(s, 1, argv);
	Object *newObject = s->_segMan->getObject(object);
	newObject->setSpeciesSelector(species);
	newObject->setSuperClassSelector(source);

	for (int i = 1; i + 1 < argc; i += 2)
		writeSelector(s->_segMan, object, argv[i].toUint16(), argv[i + 1]);
	return object;
}

reg_t kLsciResource(EngineState *s, int argc, reg_t *argv) {
	// ScummVM loads and purges resources itself, so only locks need the kernel
	const LsciResourceMode mode = (LsciResourceMode)argv[0].toUint16();
	if (mode == kLsciResourceLock && argc >= 3)
		return kLock(s, argc - 1, argv + 1);

	if (mode == kLsciResourceLoad) {
		const ResourceType type = g_sci->getResMan()->convertResType(argv[1].toUint16());
		for (int i = 2; i < argc; ++i) {
			if (!g_sci->getResMan()->testResource(ResourceId(type, argv[i].toUint16())))
				warning("kResource: %s.%d does not exist", getResourceTypeName(type), argv[i].toUint16());
		}
	}
	return s->r_acc;
}

reg_t kLsciArrayNew(EngineState *s, int argc, reg_t *argv) {
	reg_t handle;
	s->_segMan->allocateArray((SciArrayType)argv[1].toUint16(), argv[0].toUint16(), &handle);
	return handle;
}

reg_t kLsciArrayAt(EngineState *s, int argc, reg_t *argv) {
	// Out of range reads 0, and byte elements are unsigned
	if (argv[0].isNull())
		return NULL_REG;
	SciArray &array = *s->_segMan->lookupArray(argv[0]);
	const uint16 index = argv[1].toUint16();
	if (index >= array.size())
		return NULL_REG;
	if (getLsciElementSize(s->_segMan, argv[0]) == 1)
		return make_reg(0, array.byteAt(index));
	return array.IDAt(index);
}

reg_t kLsciArrayAtPut(EngineState *s, int argc, reg_t *argv) {
	if (argv[0].isNull())
		return NULL_REG;
	return kArraySetElements(s, argc, argv);
}

reg_t kLsciArrayCompare(EngineState *s, int argc, reg_t *argv) {
	const uint16 count = argv[4].toUint16();
	const uint elementSize = getLsciElementSize(s->_segMan, argv[0]);
	const Common::Array<byte> a = getLsciArrayBytes(s->_segMan, argv[0], (argv[1].toUint16() + count) * elementSize);
	const Common::Array<byte> b = getLsciArrayBytes(s->_segMan, argv[2], (argv[3].toUint16() + count) * elementSize);
	const uint aStart = argv[1].toUint16() * elementSize;
	const uint bStart = argv[3].toUint16() * elementSize;
	if (a.size() < aStart + count * elementSize || b.size() < bStart + count * elementSize)
		return NULL_REG;
	return make_reg(0, memcmp(a.data() + aStart, b.data() + bStart, count * elementSize));
}

reg_t kLsciWait(EngineState *s, int argc, reg_t *argv) {
	// With a second argument the period is set (0) or polled: the ticks since the
	// period last ended, or 0 while it runs
	if (argc < 2)
		return kWait(s, argc, argv);

	const uint32 now = g_sci->getTickCount();
	if (argv[1].isNull()) {
		s->_lsciPollPeriod = argv[0].toUint16();
		return make_reg(0, 1);
	}

	if (s->_lsciPollPeriod == 0)
		return NULL_REG;
	const uint32 elapsedTicks = now - s->_lsciLastPollTime;
	if (elapsedTicks < s->_lsciPollPeriod) {
		// Spare the host a busy loop while the scripts poll
		g_sci->sleep(1000 / 60);
		return NULL_REG;
	}
	s->_lsciLastPollTime = now;
	return make_reg(0, elapsedTicks);
}

reg_t kLsciBlock(EngineState *s, int argc, reg_t *argv) {
	// Memory blocks are arrays here; type queries and locking are left to ScummVM
	switch (argv[0].toUint16()) {
	case kLsciBlockCopy:
		return argv[1].isNull() ? NULL_REG : kArrayDuplicate(s, 1, &argv[1]);
	case kLsciBlockFree:
		if (s->_segMan->isArray(argv[1]))
			s->_segMan->freeArray(argv[1]);
		return s->r_acc;
	default:
		return s->r_acc;
	}
}

reg_t kLsciModuleDispose(EngineState *s, int argc, reg_t *argv) {
	for (int i = 0; i < argc; ++i)
		kDisposeScript(s, 1, &argv[i]);
	return s->r_acc;
}

reg_t kLsciMemory(EngineState *s, int argc, reg_t *argv) {
	// Compacting, purging and the debug dump are left to ScummVM
	switch (argv[0].toUint16()) {
	case kLsciMemoryFreeBytesLow:
	case kLsciMemoryLargestBlockLow:
		return make_reg(0, kLsciAssumedHeapBytes & 0xFFFF);
	case kLsciMemoryFreeBytesHigh:
	case kLsciMemoryLargestBlockHigh:
		return make_reg(0, kLsciAssumedHeapBytes >> 16);
	case kLsciMemoryHandleCount:
		return NULL_REG;
	case kLsciMemoryFreeHandle:
		if (argc > 1 && !argv[1].isNull())
			g_sci->_gfxPaint16->bitsFree(argv[1]);
		return s->r_acc;
	case kLsciMemoryHeapKilobytes:
		return make_reg(0, kLsciAssumedHeapBytes >> 10);
	default:
		return s->r_acc;
	}
}

reg_t kLsciStringGetToken(EngineState *s, int argc, reg_t *argv) {
	// Moves the first token of the source into the destination; 0 when none is left
	const Common::String source = getLsciString(s->_segMan, argv[0]);
	const Common::String delimiters = getLsciString(s->_segMan, argv[2]);
	const size_t start = strspn(source.c_str(), delimiters.c_str());
	const size_t length = strcspn(source.c_str() + start, delimiters.c_str());
	if (length == 0)
		return NULL_REG;

	s->_segMan->lookupArray(argv[1])->fromString(Common::String(source.c_str() + start, length));
	s->_segMan->lookupArray(argv[0])->fromString(source.c_str() + start + length);
	return argv[1];
}

static int32 getLsciLong(SegManager *segMan, reg_t object) {
	const uint16 high = readSelectorValue(segMan, object, g_sci->getKernel()->findSelector("highWord"));
	const uint16 low = readSelectorValue(segMan, object, g_sci->getKernel()->findSelector("lowWord"));
	return (int32)((high << 16) | low);
}

reg_t kLsciLong(EngineState *s, int argc, reg_t *argv) {
	// Long objects hold highWord and lowWord; the operand is a high and a low word
	SegManager *segMan = s->_segMan;
	const reg_t object = argv[1];
	int32 value = getLsciLong(segMan, object);
	const int32 operand = argc > 3 ? (int32)((argv[2].toUint16() << 16) | argv[3].toUint16()) : 0;

	switch (argv[0].toUint16()) {
	case kLsciLongAdd:
		value += operand;
		break;
	case kLsciLongSubtract:
		value -= operand;
		break;
	case kLsciLongMultiply:
		value *= operand;
		break;
	case kLsciLongDivide:
		if (operand == 0) {
			warning("kLong: division by zero");
			return object;
		}
		value /= operand;
		break;
	case kLsciLongSquareRoot:
		value = (int32)sqrt((double)ABS(value));
		break;
	case kLsciLongNegate:
		value = -value;
		break;
	case kLsciLongGreaterThan:
		return make_reg(0, value > operand);
	case kLsciLongLessThan:
		return make_reg(0, value < operand);
	case kLsciLongEquals:
		return make_reg(0, value == operand);
	case kLsciLongGreaterOrEqual:
		return make_reg(0, value >= operand);
	case kLsciLongLessOrEqual:
		return make_reg(0, value <= operand);
	case kLsciLongAsString: {
		// A non-zero flag prints the value unsigned
		const bool asUnsigned = argc > 2 && !argv[2].isNull();
		const Common::String text = asUnsigned ? Common::String::format("%u", (uint32)value) : Common::String::format("%d", value);
		return makeLsciString(segMan, text);
	}
	case kLsciLongFromString:
		value = (int32)strtol(getLsciString(segMan, argv[2]).c_str(), nullptr, 10);
		break;
	default:
		return object;
	}

	writeSelectorValue(segMan, object, g_sci->getKernel()->findSelector("highWord"), (uint32)value >> 16);
	writeSelectorValue(segMan, object, g_sci->getKernel()->findSelector("lowWord"), value & 0xFFFF);
	return object;
}

reg_t kLsciSid(EngineState *s, int argc, reg_t *argv) {
	const uint16 sid = argc > 1 ? argv[1].toUint16() : 0;
	switch (argv[0].toUint16()) {
	case kLsciSidInit:
		s->_lsciSids.clear();
		s->_lsciSidSlots = sid;
		return s->r_acc;
	case kLsciSidBind:
		// 0 when the table is full
		if (!s->_lsciSids.contains(sid) && s->_lsciSids.size() >= s->_lsciSidSlots)
			return NULL_REG;
		s->_lsciSids[sid] = argv[2];
		return make_reg(0, 1);
	case kLsciSidUnbind:
		s->_lsciSids.erase(sid);
		return s->r_acc;
	case kLsciSidLookup:
		return s->_lsciSids.getValOrDefault(sid, NULL_REG);
	default:
		warning("kSID: sub-op %d is not implemented", argv[0].toUint16());
		return NULL_REG;
	}
}

} // End of namespace Sci
