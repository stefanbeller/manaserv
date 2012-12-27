/*
 *  The Mana Server
 *  Copyright (C) 2004-2010  The Mana World Development Team
 *
 *  This file is part of The Mana Server.
 *
 *  The Mana Server is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  any later version.
 *
 *  The Mana Server is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with The Mana Server.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "game-server/actor.h"

#include "game-server/map.h"
#include "game-server/mapcomposite.h"

#include <cassert>

Actor::~Actor()
{
    // Free the map position
    if (MapComposite *mapComposite = getMap())
    {
        Map *map = mapComposite->getMap();
        Point currentTile = map->getTilePosition(mPos);
        map->freeTile(currentTile, getBlockType());
    }
}

#include <iostream>
using namespace std;

void Actor::setPosition(const Point &p)
{
    cout << "Actor::setPosition " << p.x <<" "<<p.y<<endl;

    // Update blockmap
    if (MapComposite *mapComposite = getMap())
    {
        Map *map = mapComposite->getMap();
        Point currentTile = map->getTilePosition(mPos);
        Point desiredTile = map->getTilePosition(p);
        if ( currentTile != desiredTile)
        {
            map->freeTile(currentTile, getBlockType());
            map->blockTile(desiredTile, getBlockType());
        }
    }

    mPos = p;
}

void Actor::setMap(MapComposite *mapComposite)
{
    assert(mapComposite);
    const Point p = getPosition();

    if (MapComposite *oldMapComposite = getMap())
    {
        Map *oldMap = oldMapComposite->getMap();
        Point currentTile = oldMap->getTilePosition(p);
        oldMap->freeTile(currentTile, getBlockType());
    }
    Entity::setMap(mapComposite);
    Map *map = mapComposite->getMap();
    map->blockTile(map->getTilePosition(getPosition()), getBlockType());
    /* the last line might look illogical because the current position is
     * invalid on the new map, but it is necessary to block the old position
     * because the next call of setPosition() will automatically free the old
     * position. When we don't block the position now the occupation counting
     * will be off.
     */
}
