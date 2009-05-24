/*
 *  The Mana World Server
 *  Copyright 2004 The Mana World Development Team
 *
 *  This file is part of The Mana World.
 *
 *  The Mana World is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana World is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana World; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef _TMWSERV_ACCOUNTHANDLER_H_
#define _TMWSERV_ACCOUNTHANDLER_H_

#include <string>

namespace AccountClientHandler
{
    /**
     * Creates a connection handler and starts listening on given port.
     */
    bool initialize(int port, const std::string &host);

    /**
     * Stops listening to messages and destroys the connection handler.
     */
    void deinitialize();

    /**
     * Prepares a connection for a client coming from a game server.
     */
    void prepareReconnect(const std::string &token, int accountID);

    /**
     * Processes messages received by the connection handler.
     */
    void process();
}

#endif
