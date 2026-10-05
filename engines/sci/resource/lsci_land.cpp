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

#include "common/file.h"
#include "common/textconsole.h"

#include "sci/resource/lsci_land.h"

namespace Sci {

// TSNEXEC runs each land as a TSN.PRG program; LAND.CFG names its directory
struct LsciLand {
	const char *program;
	const char *directory;
};

static const LsciLand s_lsciLands[] = {
	{ "Sierr", "" },   // Clubhouse
	{ "SLand", "SL" }, // SierraLand
	{ "LLand", "LL" }  // CasinoLand, LarryLand in TSN 2.1
};

Common::Path getLsciLandDirectory(const Common::String &program) {
	if (program.empty())
		return Common::Path();

	for (const LsciLand &land : s_lsciLands) {
		if (!program.equalsIgnoreCase(land.program))
			continue;

		const Common::Path directory(land.directory);
		if (Common::File::exists(directory.join("resource.map")))
			return directory;

		warning("LSCI land %s has no resource.map in '%s', starting in the Clubhouse", land.program, land.directory);
		return Common::Path();
	}

	warning("Unknown LSCI land program '%s', starting in the Clubhouse", program.c_str());
	return Common::Path();
}

} // End of namespace Sci
