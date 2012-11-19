/*
 *  The Mana Server
 *  Copyright (C) 2004-2011  The Mana World Development Team
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

#ifndef MAP_H
#define MAP_H

#include <list>
#include <map>
#include <string>
#include <vector>

#include "utils/logger.h"
#include "utils/point.h"
#include "utils/string.h"

typedef std::list<Point> Path;
typedef Path::iterator PathIterator;
enum BlockType
{
    BLOCKTYPE_NONE = -1,
    BLOCKTYPE_WALL,
    BLOCKTYPE_CHARACTER,
    BLOCKTYPE_MONSTER,
    NB_BLOCKTYPES
};

/**
 * A meta tile stores additional information about a location on a tile map.
 * This is information that doesn't need to be repeated for each tile in each
 * layer of the map.
 */
class MetaTile
{
    public:
        MetaTile()
            : blockmask(0)
        {
            for (unsigned i = 0; i < NB_BLOCKTYPES; ++i)
                occupation[i] = 0;
        }

        unsigned occupation[NB_BLOCKTYPES];
        char blockmask;          /**< walkability bitfield */
};

class MapObject
{
    public:
        MapObject(const Rectangle &bounds,
                  const std::string &name,
                  const std::string &type)
            : mBounds(bounds),
              mName(name),
              mType(type)
        { }

        void addProperty(const std::string &key, const std::string &value)
        {
            if (mProperties.contains(key))
                LOG_WARN("Duplicate property " << key <<
                         " of object " << mName);
            else
                mProperties.insert(key, value);
        }

        const std::string &getProperty(const std::string &key) const
        { return mProperties.value(key); }

        const std::string &getName() const
        { return mName; }

        const std::string &getType() const
        { return mType; }

        const Rectangle &getBounds() const
        { return mBounds; }

        int getX() const { return mBounds.x; }
        int getY() const { return mBounds.y; }

    private:
        Rectangle mBounds;
        std::string mName;
        std::string mType;
        utils::NameMap<std::string> mProperties;
};


/**
 * A tile map.
 */
class Map
{
    public:
        enum Orientation
        {
            MAP_UNKNOWN = -1,
            MAP_ORTHOGONAL,
            MAP_ISOMETRIC
        };

        /**
         * Constructor that takes initial map size as parameters.
         */
        Map(Orientation orientation, int width, int height,
            int tileWidth, int tileHeight);

        ~Map();

        /**
         * Sets the size of the map. This will destroy any existing map data.
         */
        void setSize(int width, int height);

        /**
         * Marks a tile as occupied, x,y in tile coordinates
         */
        void blockTile(int x, int y, BlockType type);

        /**
         * Marks a tile as occupied, x,y in tile coordinates
         */
        void blockTile(Point p, BlockType type)
            { blockTile(p.x, p.y, type); }

        /**
         * Marks a tile as unoccupied,  x,y in tile coordinates
         */
        void freeTile(int x, int y, BlockType type);

        /**
         * Marks a tile as unoccupied,  x,y in tile coordinates
         */
        void freeTile(Point p, BlockType type)
            { freeTile(p.x, p.y, type); }

        /**
         * Gets walkability for a tile with a blocking bitmask
         */
        bool getWalk(int x, int y, char walkmask = BLOCKMASK_WALL) const;

        bool getWalk(Point p, char walkmask = BLOCKMASK_WALL) const
            { return getWalk(p.x,p.y, walkmask); }

        /**
         * @brief getTilePosition converts pixel position to tile position.
         * @param pixel position in pixels.
         * @return the position in tile coordinates
         */
        Point getTilePosition(int x, int y) const;
        Point getTilePosition(Point pixel) const
        { return getTilePosition(pixel.x, pixel.y); }

        /**
         * @brief Returns the pixel position of the middle of the given tile.
         * @param pixel position in tiles.
         * @return the position in pixel coordinates
         */
        Point getTileCenter(const Point &tileCoordinates) const;

        /**
         * Tells if a tile location is within the map range.
         */
        bool contains(int x, int y) const
        { return x >= 0 && y >= 0 && x < mWidth && y < mHeight; }

        /**
         * Tells if a tile location is within the map range.
         */
        bool contains(const Point &p) const
        { return p.x >= 0 && p.y >= 0 && p.x < mWidth && p.y < mHeight; }

        /**
         * Returns the width of this map.
         */
        int getWidth() const
        { return mWidth; }

        /**
         * Returns the height of this map.
         */
        int getHeight() const
        { return mHeight; }

        /**
         * Returns the tile width of this map.
         */
        int getTileWidth() const
        { return mTileWidth; }

        /**
         * Returns the tile height used by this map.
         */
        int getTileHeight() const
        { return mTileHeight; }

        /**
         * Returns a general map property defined in the map file
         */
        const std::string &getProperty(const std::string &key) const;

        /**
        * Sets a map property
        */
        void setProperty(const std::string &key, const std::string &val)
        { mProperties[key] = val; }

        /**
         * Adds an object.
         */
        void addObject(MapObject *object)
        { mMapObjects.push_back(object); }

        /**
         * Returns the objects of the map.
         */
        const std::vector<MapObject*> &getObjects() const
        { return mMapObjects; }

        /**
         * Find a path from one location to the next.
         */
        Path findPath(int startX, int startY,
                      int destX, int destY,
                      unsigned char walkmask,
                      int maxCost = 20) const;

        /**
         * Find a path from one location to the next.
         */
        Path findPath(const Point &start, const Point &dest,
                      unsigned char walkmask,
                      int maxCost = 20) const
            { return findPath(start.x, start.y, dest.x, dest.y, walkmask, maxCost); }

        /**
         * Blockmasks for different entities
         */
        static const unsigned char BLOCKMASK_WALL = 0x80;     // = bin 1000 0000
        static const unsigned char BLOCKMASK_CHARACTER = 0x01;// = bin 0000 0001
        static const unsigned char BLOCKMASK_MONSTER = 0x02;  // = bin 0000 0010

    private:
        // map properties
        Orientation mOrientation;
        int mWidth, mHeight;
        int mTileWidth, mTileHeight;
        std::map<std::string, std::string> mProperties;

        std::vector<MetaTile> mMetaTiles;
        std::vector<MapObject*> mMapObjects;
};

#endif
