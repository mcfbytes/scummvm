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

#include "common/endian.h"

#include "sci/engine/lsci_script_image.h"
#include "sci/engine/object.h"
#include "sci/engine/script.h"
#include "sci/engine/vm.h"

namespace Sci {

enum {
	kBlockHeaderSize = 4,
	kObjectHeaderSize = 2 + kOffsetHeaderSize, ///< magic word, then the header Object::init reads
	kObjectPropertiesOffset = kBlockHeaderSize + kObjectHeaderSize,
	kImageLimit = 0x10000,
	kNoOffset = 0xFFFFFFFF
};

// LSCI opcodes that differ from SCI opcodes of the same number
enum LsciOpcode {
	kLsciOpLine = op_info,
	kLsciOpFile = op_superP,
	kLsciOpLoadId = op_lea,
	kLsciOpPushId = op_lofss
};

// Operands of every LSCI opcode, as the original interpreter reads them
static const opcode_format s_lsciOperandFormats[128][4] = {
	// 00 - 03 / bnot, add, sub, mul
	{Script_None}, {Script_None}, {Script_None}, {Script_None},
	// 04 - 07 / div, mod, shr, shl
	{Script_None}, {Script_None}, {Script_None}, {Script_None},
	// 08 - 0B / xor, and, or, neg
	{Script_None}, {Script_None}, {Script_None}, {Script_None},
	// 0C - 0F / not, eq, ne, gt
	{Script_None}, {Script_None}, {Script_None}, {Script_None},
	// 10 - 13 / ge, lt, le, ugt
	{Script_None}, {Script_None}, {Script_None}, {Script_None},
	// 14 - 17 / uge, ult, ule, bt
	{Script_None}, {Script_None}, {Script_None}, {Script_SRelative},
	// 18 - 1B / bnt, jmp, ldi, push
	{Script_SRelative}, {Script_SRelative}, {Script_SVariable}, {Script_None},
	// 1C - 1F / pushi, toss, dup, link
	{Script_SVariable}, {Script_None}, {Script_None}, {Script_Variable},
	// 20 - 23 / call, callk, callb, calle
	{Script_Word, Script_Byte}, {Script_Variable, Script_Byte}, {Script_Variable, Script_Byte}, {Script_Variable, Script_Byte, Script_Byte},
	// 24 - 27 / ret, send, _line_, _file_
	{Script_End}, {Script_Byte}, {Script_Word}, {Script_Word},
	// 28 - 2B / class, dummy, self, super
	{Script_Variable}, {Script_Invalid}, {Script_Byte}, {Script_Variable, Script_Byte},
	// 2C - 2F / &rest, loadID, selfID, dummy
	{Script_Byte}, {Script_SVariable}, {Script_None}, {Script_Invalid},
	// 30 - 33 / pprev, pToa, aTop, pTos
	{Script_None}, {Script_Property}, {Script_Property}, {Script_Property},
	// 34 - 37 / sTop, ipToa, dpToa, ipTos
	{Script_Property}, {Script_Property}, {Script_Property}, {Script_Property},
	// 38 - 3B / dpTos, lofsa (invalid), pushID, push0
	{Script_Property}, {Script_Invalid}, {Script_SVariable}, {Script_None},
	// 3C - 3F / push1, push2, pushSelf, line (invalid)
	{Script_None}, {Script_None}, {Script_None}, {Script_Invalid},
	// 40 - 4F / lag ... lspi
	{Script_Global}, {Script_Local}, {Script_Temp}, {Script_Param},
	{Script_Global}, {Script_Local}, {Script_Temp}, {Script_Param},
	{Script_Global}, {Script_Local}, {Script_Temp}, {Script_Param},
	{Script_Global}, {Script_Local}, {Script_Temp}, {Script_Param},
	// 50 - 5F / sag ... sspi
	{Script_Global}, {Script_Local}, {Script_Temp}, {Script_Param},
	{Script_Global}, {Script_Local}, {Script_Temp}, {Script_Param},
	{Script_Global}, {Script_Local}, {Script_Temp}, {Script_Param},
	{Script_Global}, {Script_Local}, {Script_Temp}, {Script_Param},
	// 60 - 6F / plusag ... plusspi
	{Script_Global}, {Script_Local}, {Script_Temp}, {Script_Param},
	{Script_Global}, {Script_Local}, {Script_Temp}, {Script_Param},
	{Script_Global}, {Script_Local}, {Script_Temp}, {Script_Param},
	{Script_Global}, {Script_Local}, {Script_Temp}, {Script_Param},
	// 70 - 7F / minusag ... minusspi
	{Script_Global}, {Script_Local}, {Script_Temp}, {Script_Param},
	{Script_Global}, {Script_Local}, {Script_Temp}, {Script_Param},
	{Script_Global}, {Script_Local}, {Script_Temp}, {Script_Param},
	{Script_Global}, {Script_Local}, {Script_Temp}, {Script_Param}
};

static uint32 getOperandSize(opcode_format format, bool byteForm) {
	switch (format) {
	case Script_Byte:
	case Script_SByte:
		return 1;
	case Script_Word:
	case Script_SWord:
		return 2;
	case Script_None:
	case Script_End:
		return 0;
	default:
		return byteForm ? 1 : 2;
	}
}

static bool isObjectItem(const LsciContainer::Item &item) {
	return item.tag == kLsciItemObject || item.tag == kLsciItemClass;
}

static uint32 getBodySize(const LsciContainer::Item &item) {
	switch (item.tag) {
	case kLsciItemObject:
	case kLsciItemClass: {
		const uint32 propertyWords = item.getPropertyCount() * (item.tag == kLsciItemClass ? 2 : 1);
		return kObjectHeaderSize + (propertyWords + 2 + item.getMethodCount() * 2) * 2;
	}
	case kLsciItemString:
		return item.payload.size() - kLsciStringTextOffset;
	case kLsciItemDispatchTable:
		return (1 + (item.getWord(0) + 1) * 2) * 2;
	default:
		return item.payload.size();
	}
}

// Blocks are padded to an even size, and the size in their header includes the padding
static uint32 getBlockSize(const LsciContainer::Item &item) {
	const uint32 size = kBlockHeaderSize + getBodySize(item);
	return size + (size & 1);
}

bool LsciScriptImageWriter::write(const LsciContainer &container, Common::Array<byte> &image, Common::String &error) {
	_container = &container;
	_image = &image;
	_blockOffsets.clear();
	_itemOffsets.clear();
	_relocations.clear();
	_strayBranches.clear();
	image.clear();

	if (!layout(error) || !checkReferences(error))
		return false;

	for (uint index = 0; index < container.items.size(); ++index) {
		if (_blockOffsets[index] != kNoOffset && !writeBlock(index, error))
			return false;
	}
	writeRelocations();

	if (image.size() > kImageLimit) {
		error = Common::String::format("image of %u bytes exceeds a 16-bit segment", image.size());
		return false;
	}
	return true;
}

// LSCI uses the last Variables item as the script's locals; earlier ones get no block
static uint findLocalsItem(const LsciContainer &container) {
	uint localsItem = container.items.size();
	for (uint index = 0; index < container.items.size(); ++index) {
		if (container.items[index].tag == kLsciItemVariables)
			localsItem = index;
	}
	return localsItem;
}

bool LsciScriptImageWriter::layout(Common::String &error) {
	const uint localsItem = findLocalsItem(*_container);
	uint32 position = 0;
	for (uint index = 0; index < _container->items.size(); ++index) {
		const LsciContainer::Item &item = _container->items[index];
		const bool hasBlock = item.tag != kLsciItemPropertyDictionary &&
			(item.tag != kLsciItemVariables || index == localsItem);
		if (!hasBlock) {
			_blockOffsets.push_back(kNoOffset);
			_itemOffsets.push_back(kNoOffset);
			continue;
		}
		_blockOffsets.push_back(position);
		_itemOffsets.push_back(position + (isObjectItem(item) ? kObjectPropertiesOffset : kBlockHeaderSize));
		position += getBlockSize(item);
		if (position > kImageLimit) {
			error = Common::String::format("item %u ends beyond a 16-bit segment", index);
			return false;
		}
	}
	_image->resize(position);
	return true;
}

bool LsciScriptImageWriter::checkReferences(Common::String &error) const {
	for (uint index = 0; index < _container->items.size(); ++index) {
		const LsciContainer::Item &item = _container->items[index];
		for (uint i = 0; i < item.fixups.size(); ++i) {
			const uint16 target = item.getWord(item.fixups[i]);
			if (_itemOffsets[target] == kNoOffset) {
				error = Common::String::format("item %u: reference to item %u, which has no block", index, target);
				return false;
			}
		}
	}
	return true;
}

bool LsciScriptImageWriter::writeBlock(uint index, Common::String &error) {
	const LsciContainer::Item &item = _container->items[index];
	const uint32 blockOffset = _blockOffsets[index];
	const uint32 bodyOffset = blockOffset + kBlockHeaderSize;
	ScriptObjectTypes type;

	switch (item.tag) {
	case kLsciItemCode:
		type = SCI_OBJ_CODE;
		if (!writeCode(index, bodyOffset, error))
			return false;
		break;
	case kLsciItemObject:
	case kLsciItemClass:
		type = item.tag == kLsciItemClass ? SCI_OBJ_CLASS : SCI_OBJ_OBJECT;
		writeObject(index, bodyOffset);
		break;
	case kLsciItemString:
		type = SCI_OBJ_STRINGS;
		copyBytes(bodyOffset, item.payload.subspan(kLsciStringTextOffset));
		break;
	case kLsciItemVariables:
		type = SCI_OBJ_LOCALVARS;
		writeVariables(index, bodyOffset);
		break;
	default:
		type = SCI_OBJ_EXPORTS;
		writeExports(index, bodyOffset);
		break;
	}

	setWord(blockOffset, type);
	setWord(blockOffset + 2, getBlockSize(item));
	return true;
}

void LsciScriptImageWriter::writeObject(uint index, uint32 bodyOffset) {
	const LsciContainer::Item &item = _container->items[index];
	const uint16 propertyCount = item.getPropertyCount();
	const uint16 methodCount = item.getMethodCount();
	const uint32 properties = bodyOffset + kObjectHeaderSize;

	for (uint16 slot = 0; slot < propertyCount; ++slot) {
		const uint32 imageOffset = properties + slot * 2;
		// An instance's species is its class, as the original copies the class's dictionary into it
		if (slot == kLsciSlotDict && item.tag == kLsciItemObject)
			setWord(imageOffset, item.getWord(kLsciSlotSuper * 2));
		else
			setWord(imageOffset, resolveWord(item, slot * 2, imageOffset));
	}

	uint32 methodBlock = properties + propertyCount * 2;
	if (item.tag == kLsciItemClass) {
		const LsciContainer::Item &dictionary = _container->items[index + 1];
		for (uint16 slot = 0; slot < propertyCount; ++slot, methodBlock += 2)
			setWord(methodBlock, dictionary.getWord(slot * 2));
	}

	setWord(bodyOffset, SCRIPT_OBJECT_MAGIC_NUMBER);
	setWord(bodyOffset + 2 + kOffsetHeaderFunctionArea, methodBlock + 2 - properties);
	setWord(bodyOffset + 2 + kOffsetHeaderSelectorCounter, propertyCount);

	// SCI0 method blocks have a zero word between the selectors and the code offsets
	const uint32 selectors = (propertyCount + 1) * 2;
	const uint32 codeOffsets = selectors + methodCount * 2;
	setWord(methodBlock, methodCount);
	for (uint16 method = 0; method < methodCount; ++method) {
		setWord(methodBlock + 2 + method * 2, item.getWord(selectors + method * 2));
		setWord(methodBlock + 4 + (methodCount + method) * 2, resolveWord(item, codeOffsets + method * 2, kNoOffset));
	}
}

bool LsciScriptImageWriter::writeCode(uint index, uint32 bodyOffset, Common::String &error) {
	const LsciContainer::Item &item = _container->items[index];
	const uint32 length = item.payload.size();
	copyBytes(bodyOffset, item.payload);

	uint references = 0;
	for (uint32 pc = 0; pc < length;) {
		const byte opcode = item.payload[pc] >> 1;
		const bool byteForm = item.payload[pc] & 1;
		const opcode_format *formats = s_lsciOperandFormats[opcode];
		if (formats[0] == Script_Invalid) {
			error = Common::String::format("item %u: invalid opcode %02x at %04x", index, item.payload[pc], pc);
			return false;
		}

		uint32 size = 1;
		for (uint i = 0; i < 4 && formats[i]; ++i)
			size += getOperandSize(formats[i], byteForm);
		if (pc + size > length) {
			error = Common::String::format("item %u: operands at %04x run past the end", index, pc);
			return false;
		}
		if (!rewriteInstruction(index, bodyOffset, pc, size, references, error))
			return false;

		if (opcode == op_bt || opcode == op_bnt || opcode == op_jmp) {
			const int32 relative = byteForm ? (int8)item.payload[pc + 1] : (int16)item.getWord(pc + 1);
			const int32 target = (int32)(pc + size) + relative;
			if (target < 0 || target >= (int32)length) {
				const StrayBranch stray = { (uint16)index, (uint16)pc, target };
				_strayBranches.push_back(stray);
			}
		}
		pc += size;
	}

	if (references != item.fixups.size()) {
		error = Common::String::format("item %u: %u fixups are not operands of call, loadID or pushID", index, item.fixups.size() - references);
		return false;
	}
	return true;
}

bool LsciScriptImageWriter::rewriteInstruction(uint index, uint32 bodyOffset, uint32 pc, uint32 size, uint &references, Common::String &error) {
	const LsciContainer::Item &item = _container->items[index];
	const byte opcode = item.payload[pc] >> 1;
	const bool byteForm = item.payload[pc] & 1;
	const uint32 operand = pc + 1;
	byte &imageOpcode = (*_image)[bodyOffset + pc];

	switch (opcode) {
	case op_call:
		if (!item.isReference(operand)) {
			error = Common::String::format("item %u: call at %04x names no item", index, pc);
			return false;
		}
		imageOpcode = op_call << 1;
		setWord(bodyOffset + operand, getTarget(item, operand) - (bodyOffset + pc + size));
		++references;
		break;
	case kLsciOpLoadId:
	case kLsciOpPushId:
		if (!byteForm && item.isReference(operand)) {
			imageOpcode = (opcode == kLsciOpLoadId ? op_lofsa : op_lofss) << 1;
			setWord(bodyOffset + operand, getTarget(item, operand));
			++references;
		} else {
			imageOpcode = (opcode == kLsciOpLoadId ? op_ldi : op_pushi) << 1 | byteForm;
		}
		break;
	case kLsciOpLine:
	case kLsciOpFile:
		imageOpcode = op_line << 1;
		break;
	case op_pushSelf:
		imageOpcode = op_pushSelf << 1;
		break;
	default:
		break;
	}
	return true;
}

void LsciScriptImageWriter::writeVariables(uint index, uint32 bodyOffset) {
	const LsciContainer::Item &item = _container->items[index];
	for (uint32 offset = 0; offset + 1 < item.payload.size(); offset += 2)
		setWord(bodyOffset + offset, resolveWord(item, offset, bodyOffset + offset));
}

void LsciScriptImageWriter::writeExports(uint index, uint32 bodyOffset) {
	const LsciContainer::Item &item = _container->items[index];
	const uint16 count = item.getWord(0) + 1;
	setWord(bodyOffset, count);
	for (uint16 entry = 0; entry < count; ++entry) {
		const uint32 offset = (entry + 1) * 2;
		setWord(bodyOffset + 2 + entry * 4, item.isReference(offset) ? getTarget(item, offset) : 0);
	}
}

void LsciScriptImageWriter::writeRelocations() {
	const uint32 blockOffset = _image->size();
	const uint32 blockSize = kBlockHeaderSize + (1 + _relocations.size()) * 2;
	_image->resize(blockOffset + blockSize + 2);
	setWord(blockOffset, SCI_OBJ_POINTERS);
	setWord(blockOffset + 2, blockSize);
	setWord(blockOffset + 4, _relocations.size());
	for (uint i = 0; i < _relocations.size(); ++i)
		setWord(blockOffset + 6 + i * 2, _relocations[i]);
	setWord(blockOffset + blockSize, SCI_OBJ_TERMINATOR);
}

uint16 LsciScriptImageWriter::getTarget(const LsciContainer::Item &item, uint32 payloadOffset) const {
	return _itemOffsets[item.getWord(payloadOffset)];
}

uint16 LsciScriptImageWriter::resolveWord(const LsciContainer::Item &item, uint32 payloadOffset, uint32 imageOffset) {
	if (!item.isReference(payloadOffset))
		return item.getWord(payloadOffset);
	if (imageOffset != kNoOffset)
		_relocations.push_back(imageOffset);
	return getTarget(item, payloadOffset);
}

void LsciScriptImageWriter::copyBytes(uint32 imageOffset, const SciSpan<const byte> &bytes) {
	if (bytes.size())
		memcpy(&(*_image)[imageOffset], bytes.getUnsafeDataAt(0, bytes.size()), bytes.size());
}

void LsciScriptImageWriter::setWord(uint32 imageOffset, uint16 value) {
	WRITE_LE_UINT16(&(*_image)[imageOffset], value);
}

} // End of namespace Sci
