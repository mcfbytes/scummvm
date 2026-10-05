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
#include "common/textconsole.h"

#include "sci/engine/tsn_online_executive.h"
#include "sci/engine/tsn_offline_executive.h"
#ifdef USE_BASIC_NET
#include "sci/engine/tsn_socket_transport.h"
#endif

namespace Sci {

enum {
	kInnkeeperdTransportPort = 2315
};

TsnExecutive *makeTsnExecutive() {
	const Common::String host = ConfMan.get("lsci_host");
	if (host.empty())
		return new OfflineTsnExecutive();

#ifdef USE_BASIC_NET
	const int port = ConfMan.hasKey("lsci_port") ? ConfMan.getInt("lsci_port") : (int)kInnkeeperdTransportPort;
	return new OnlineTsnExecutive(new TsnSocketTransport(host, port));
#else
	warning("lsci_host is set, but this build has no basic networking: the TSN executive stays offline");
	return new OfflineTsnExecutive();
#endif
}

} // End of namespace Sci
