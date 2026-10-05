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
#include "sci/engine/lsci_kernel.h"
#include "sci/engine/seg_manager.h"
#include "sci/engine/state.h"

namespace Sci {

// LSCI lists are vectors of values, held here in SCI32 ID arrays whose size is
// the count; LSCI iterators are ID arrays laid out as below.
enum LsciIterator {
	kLsciIteratorPosition = 0,
	kLsciIteratorMode = 1,
	kLsciIteratorCount = 2,
	kLsciIteratorList = 3,
	kLsciIteratorSnapshot = 4
};

enum {
	kLsciIteratorLive = 1
};

static Common::Array<reg_t> getLsciList(SegManager *segMan, reg_t list) {
	Common::Array<reg_t> values;
	if (list.isNull())
		return values;
	SciArray &array = *segMan->lookupArray(list);
	for (uint16 i = 0; i < array.size(); ++i)
		values.push_back(array.IDAt(i));
	return values;
}

static void setLsciList(SegManager *segMan, reg_t list, const Common::Array<reg_t> &values) {
	SciArray &array = *segMan->lookupArray(list);
	array.resize(values.size(), true);
	if (!values.empty())
		array.setElements(0, values.size(), values.data());
}

static int findLsciListValue(const Common::Array<reg_t> &values, reg_t value) {
	for (uint i = 0; i < values.size(); ++i) {
		if (values[i] == value)
			return i;
	}
	return -1;
}

reg_t kLsciList(EngineState *s, int argc, reg_t *argv) {
	if (!s)
		return make_reg(0, getSciVersion());
	error("not supposed to call this");
}

reg_t kLsciListNew(EngineState *s, int argc, reg_t *argv) {
	// The capacity only sizes the original's first allocation
	reg_t list;
	s->_segMan->allocateArray(kArrayTypeID, 0, &list);
	return list;
}

reg_t kLsciListDispose(EngineState *s, int argc, reg_t *argv) {
	if (!argv[0].isNull())
		s->_segMan->freeArray(argv[0]);
	return s->r_acc;
}

reg_t kLsciListCount(EngineState *s, int argc, reg_t *argv) {
	return make_reg(0, getLsciList(s->_segMan, argv[0]).size());
}

reg_t kLsciListAt(EngineState *s, int argc, reg_t *argv) {
	const Common::Array<reg_t> values = getLsciList(s->_segMan, argv[0]);
	const int16 index = argv[1].toSint16();
	if (index < 0 || index > (int16)values.size())
		warning("kList(At): index %d out of range", index);
	if (index < 0 || index >= (int16)values.size())
		return NULL_REG;
	return values[index];
}

reg_t kLsciListDelete(EngineState *s, int argc, reg_t *argv) {
	Common::Array<reg_t> values = getLsciList(s->_segMan, argv[0]);
	const int index = findLsciListValue(values, argv[1]);
	if (index < 0)
		return NULL_REG;
	values.remove_at(index);
	setLsciList(s->_segMan, argv[0], values);
	return argv[1];
}

reg_t kLsciListContains(EngineState *s, int argc, reg_t *argv) {
	if (findLsciListValue(getLsciList(s->_segMan, argv[0]), argv[1]) < 0)
		return NULL_REG;
	return argv[1];
}

reg_t kLsciListAddToFront(EngineState *s, int argc, reg_t *argv) {
	if (!argv[0].isNull()) {
		Common::Array<reg_t> values = getLsciList(s->_segMan, argv[0]);
		values.insert_at(0, argv[1]);
		setLsciList(s->_segMan, argv[0], values);
	}
	return argv[1];
}

reg_t kLsciListAddToEnd(EngineState *s, int argc, reg_t *argv) {
	if (!argv[0].isNull()) {
		Common::Array<reg_t> values = getLsciList(s->_segMan, argv[0]);
		values.push_back(argv[1]);
		setLsciList(s->_segMan, argv[0], values);
	}
	return argv[1];
}

static void addLsciListValueNextTo(SegManager *segMan, reg_t list, reg_t key, reg_t value, int offset) {
	Common::Array<reg_t> values = getLsciList(segMan, list);
	const int index = findLsciListValue(values, key);
	if (index < 0)
		return;
	values.insert_at(index + offset, value);
	setLsciList(segMan, list, values);
}

reg_t kLsciListAddAfter(EngineState *s, int argc, reg_t *argv) {
	addLsciListValueNextTo(s->_segMan, argv[0], argv[1], argv[2], 1);
	return argv[2];
}

reg_t kLsciListAddBefore(EngineState *s, int argc, reg_t *argv) {
	addLsciListValueNextTo(s->_segMan, argv[0], argv[1], argv[2], 0);
	return argv[2];
}

reg_t kLsciListMoveToFront(EngineState *s, int argc, reg_t *argv) {
	Common::Array<reg_t> values = getLsciList(s->_segMan, argv[0]);
	const int index = findLsciListValue(values, argv[1]);
	if (index < 0)
		return NULL_REG;
	values.remove_at(index);
	values.insert_at(0, argv[1]);
	setLsciList(s->_segMan, argv[0], values);
	return argv[1];
}

reg_t kLsciListMoveToEnd(EngineState *s, int argc, reg_t *argv) {
	Common::Array<reg_t> values = getLsciList(s->_segMan, argv[0]);
	const int index = findLsciListValue(values, argv[1]);
	if (index < 0)
		return NULL_REG;
	values.remove_at(index);
	values.push_back(argv[1]);
	setLsciList(s->_segMan, argv[0], values);
	return argv[1];
}

reg_t kLsciListIndexOf(EngineState *s, int argc, reg_t *argv) {
	return make_reg(0, findLsciListValue(getLsciList(s->_segMan, argv[0]), argv[1]));
}

reg_t kLsciListRelease(EngineState *s, int argc, reg_t *argv) {
	if (!argv[0].isNull())
		setLsciList(s->_segMan, argv[0], Common::Array<reg_t>());
	return argv[0];
}

// Binds an iterator to a list. A live iterator reads the list itself, a
// snapshot one a copy; both stop at the count the list had when bound.
static void bindLsciIterator(SegManager *segMan, reg_t iterator, reg_t list) {
	SciArray &array = *segMan->lookupArray(iterator);
	const Common::Array<reg_t> values = getLsciList(segMan, list);
	array.resize(kLsciIteratorSnapshot, true);
	array.IDAt(kLsciIteratorPosition) = make_reg(0, -1);
	array.IDAt(kLsciIteratorCount) = make_reg(0, values.size());
	array.IDAt(kLsciIteratorList) = list;
	if (array.IDAt(kLsciIteratorMode).toUint16() != kLsciIteratorLive && !values.empty())
		array.setElements(kLsciIteratorSnapshot, values.size(), values.data());
}

static reg_t getLsciIteratorValue(SegManager *segMan, SciArray &iterator, int16 position) {
	if (iterator.IDAt(kLsciIteratorMode).toUint16() != kLsciIteratorLive)
		return iterator.IDAt(kLsciIteratorSnapshot + position);
	const Common::Array<reg_t> values = getLsciList(segMan, iterator.IDAt(kLsciIteratorList));
	return position < (int16)values.size() ? values[position] : NULL_REG;
}

reg_t kLsciSeq(EngineState *s, int argc, reg_t *argv) {
	if (!s)
		return make_reg(0, getSciVersion());
	error("not supposed to call this");
}

reg_t kLsciSeqNew(EngineState *s, int argc, reg_t *argv) {
	reg_t iterator;
	SciArray &array = *s->_segMan->allocateArray(kArrayTypeID, kLsciIteratorSnapshot, &iterator);
	array.IDAt(kLsciIteratorMode) = make_reg(0, argc > 1 ? argv[1].toUint16() : (uint16)kLsciIteratorLive);
	bindLsciIterator(s->_segMan, iterator, argv[0]);
	return iterator;
}

reg_t kLsciSeqDispose(EngineState *s, int argc, reg_t *argv) {
	if (!argv[0].isNull())
		s->_segMan->freeArray(argv[0]);
	return s->r_acc;
}

reg_t kLsciSeqBegin(EngineState *s, int argc, reg_t *argv) {
	SciArray &iterator = *s->_segMan->lookupArray(argv[0]);
	iterator.IDAt(kLsciIteratorPosition) = make_reg(0, -1);
	return s->r_acc;
}

reg_t kLsciSeqEnd(EngineState *s, int argc, reg_t *argv) {
	SciArray &iterator = *s->_segMan->lookupArray(argv[0]);
	iterator.IDAt(kLsciIteratorPosition) = iterator.IDAt(kLsciIteratorCount);
	return s->r_acc;
}

reg_t kLsciSeqNext(EngineState *s, int argc, reg_t *argv) {
	SciArray &iterator = *s->_segMan->lookupArray(argv[0]);
	const int16 position = iterator.IDAt(kLsciIteratorPosition).toSint16() + 1;
	const int16 count = iterator.IDAt(kLsciIteratorCount).toSint16();
	if (position >= count) {
		iterator.IDAt(kLsciIteratorPosition) = make_reg(0, count);
		return NULL_REG;
	}
	iterator.IDAt(kLsciIteratorPosition) = make_reg(0, position);
	return getLsciIteratorValue(s->_segMan, iterator, position);
}

reg_t kLsciSeqPrev(EngineState *s, int argc, reg_t *argv) {
	SciArray &iterator = *s->_segMan->lookupArray(argv[0]);
	const int16 position = iterator.IDAt(kLsciIteratorPosition).toSint16() - 1;
	if (position < 0) {
		iterator.IDAt(kLsciIteratorPosition) = make_reg(0, -1);
		return NULL_REG;
	}
	iterator.IDAt(kLsciIteratorPosition) = make_reg(0, position);
	return getLsciIteratorValue(s->_segMan, iterator, position);
}

reg_t kLsciSeqOver(EngineState *s, int argc, reg_t *argv) {
	bindLsciIterator(s->_segMan, argv[0], argv[1]);
	return s->r_acc;
}

} // End of namespace Sci
