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
#include "common/fs.h"

#include "sci/sci.h"
#include "sci/engine/file.h"
#include "sci/engine/kernel.h"
#include "sci/engine/lsci_array.h"
#include "sci/engine/lsci_kernel.h"
#include "sci/engine/selector.h"
#include "sci/engine/state.h"

namespace Sci {

enum {
	kLsciDirectoryDepth = 4
};

// LSCI's FileSystem kernel on ScummVM's script files: the File class passes
// handles, except to the sub-ops that take the File object itself.

static FileHandle *getLsciFile(EngineState *s, reg_t handleOrObject) {
	if (s->_segMan->isObject(handleOrObject))
		handleOrObject = readSelector(s->_segMan, handleOrObject, g_sci->getKernel()->findSelector("handle"));
	return getFileFromHandle(s, handleOrObject.toUint16());
}

// The data of a Str object, or the string itself
static reg_t getLsciStringHandle(EngineState *s, reg_t stringOrObject) {
	if (s->_segMan->isObject(stringOrObject))
		return readSelector(s->_segMan, stringOrObject, SELECTOR(data));
	return stringOrObject;
}

reg_t kLsciFileOpen(EngineState *s, int argc, reg_t *argv) {
	// The scripts may prefix LSCI.CFG's pathStr; ScummVM keeps the files by name
	Common::String name = getLsciString(s->_segMan, argv[0]);
	const size_t separator = name.findLastOf("\\/:");
	if (separator != Common::String::npos)
		name = name.substr(separator + 1);
	return file_open(s, name, (kFileOpenMode)argv[1].toUint16(), true);
}

reg_t kLsciFileWriteString(EngineState *s, int argc, reg_t *argv) {
	reg_t args[2] = { argv[0], getLsciStringHandle(s, argv[1]) };
	return kFileIOWriteString(s, 2, args);
}

reg_t kLsciFileReadString(EngineState *s, int argc, reg_t *argv) {
	// Scripts read the fields of a record by length, and the last length can be 0
	if (argv[1].isNull())
		return NULL_REG;

	reg_t args[3] = { getLsciStringHandle(s, argv[0]), argv[1], argv[2] };
	return kFileIOReadString(s, 3, args).isNull() ? NULL_REG : argv[0];
}

reg_t kLsciFileGetByte(EngineState *s, int argc, reg_t *argv) {
	FileHandle *file = getLsciFile(s, argv[0]);
	if (!file || !file->_in)
		return SIGNAL_REG;
	const byte value = file->_in->readByte();
	return file->_in->eos() ? SIGNAL_REG : make_reg(0, value);
}

reg_t kLsciFileSeek(EngineState *s, int argc, reg_t *argv) {
	FileHandle *file = getLsciFile(s, argv[0]);
	if (!file || !file->_in)
		return SIGNAL_REG;
	const int32 offset = (argv[2].toUint16() << 16) | argv[1].toUint16();
	file->_in->seek(offset, argc > 3 ? argv[3].toUint16() : SEEK_SET);
	return make_reg(0, file->_in->pos() & 0xFFFF);
}

reg_t kLsciFileReadToArray(EngineState *s, int argc, reg_t *argv) {
	FileHandle *file = getLsciFile(s, argv[0]);
	if (!file || !file->_in)
		return NULL_REG;
	Common::Array<byte> bytes(argv[2].toUint16());
	bytes.resize(file->_in->read(bytes.data(), bytes.size()));
	setLsciArrayBytes(s->_segMan, argv[1], bytes);
	return make_reg(0, bytes.size());
}

reg_t kLsciFileWriteFromArray(EngineState *s, int argc, reg_t *argv) {
	FileHandle *file = getLsciFile(s, argv[0]);
	if (!file || !file->_out)
		return NULL_REG;
	const Common::Array<byte> bytes = getLsciArrayBytes(s->_segMan, argv[1], argv[2].toUint16());
	return make_reg(0, file->_out->write(bytes.data(), bytes.size()));
}

reg_t kLsciFileSize(EngineState *s, int argc, reg_t *argv) {
	FileHandle *file = getLsciFile(s, argv[0]);
	return make_reg(0, file && file->_in ? file->_in->size() & 0xFFFF : 0);
}

reg_t kLsciFileSizeHigh(EngineState *s, int argc, reg_t *argv) {
	FileHandle *file = getLsciFile(s, argv[0]);
	return make_reg(0, file && file->_in ? file->_in->size() >> 16 : 0);
}

// A DOS path relative to the game directory, the working directory of LSCITV
reg_t kLsciFileIsDirectory(EngineState *s, int argc, reg_t *argv) {
	const Common::Path path = Common::Path(getLsciString(s->_segMan, argv[0]), '\\').normalize();
	if (path.empty())
		return TRUE_REG;

	Common::FSDirectory gameDirectory(ConfMan.getPath("path"), kLsciDirectoryDepth);
	Common::ScopedPtr<Common::FSDirectory> directory(gameDirectory.getSubDirectory(path));
	return make_reg(0, directory.get() != nullptr);
}

} // End of namespace Sci
