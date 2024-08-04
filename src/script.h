/*
    vlamits2-runtime-sdl
    Copyright (C) 2024 Francois Blanchette

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef __SCRIPT_H
#define __SCRIPT_H
#include <string>
#include <cstdint>
#include <cstdio>
#include <memory>
#include "defs.h"
#include "actor.h"

class CActor;

class CScript
{
public:
    enum
    {
        UP,
        DOWN,
        LEFT,
        RIGHT,
        NOT_FOUND = -1
    };

    CScript();
    CScript(std::unique_ptr<CActor[]> &script, uint32_t size);
    ~CScript();

    void forget();
    bool write(FILE *tfile);
    bool read(FILE *sfile);
    bool fromMemory(const uint8_t *data);
    std::string name() const;
    void setName(const std::string &name);
    std::string tileset() const;
    void setTileSet(const std::string &tileset);
    void shift(int aim);

    constexpr inline int getSize() const
    {
        return m_size;
    }
    inline CActor &operator[](int i)
    {
        return m_script.get()[i];
    }
    constexpr static inline uint32_t toKey(const uint8_t x, const uint8_t y)
    {
        return x + (y << 16);
    }
    constexpr static inline bool isBackgroundType(const uint8_t type)
    {
        return type == TYPE_BLANK || type >= TYPE_LADDER;
    }
    constexpr static inline bool isForegroundType(const uint8_t type)
    {
        return type != TYPE_BLANK && type < TYPE_LADDER;
    }
    constexpr static inline bool isMonsterType(const uint8_t type)
    {
        return (type & TYPE_FILTER_GROUP) == TYPE_MONSTER_FILTER;
    }
    constexpr static inline bool isPlayerType(const uint8_t type)
    {
        return type == TYPE_PLAYER;
    }
    constexpr static inline bool isObjectType(const uint8_t type)
    {
        return (type <= TYPE_OBJECT_SUBGROUP) &&
               (type != TYPE_PLAYER) &&
               (type != TYPE_BLANK);
    }
    constexpr static inline bool isSystemType(const uint8_t type)
    {
        return type == TYPE_STOPCLASS || type == TYPE_EMPTY;
    }
    int add(const CActor &entry);
    int insertAt(uint32_t i, const CActor &entry);
    void removeAt(int i);
    inline CActor &at(int i)
    {
        return (*this)[i];
    }
    inline const CActor &peekAt(int i) const
    {
        return m_script.get()[i];
    }
    int findPlayerIndex() const;
    int countType(uint8_t type) const;
    void sort();
    uint32_t rgbaColor();
    void setRgbaColor(uint32_t color);
    enum
    {
        TILESET_NAME_MAX = 12,
    };

private:
    uint32_t m_rgbaColor;
    std::string m_name;
    std::string m_tileset;
    std::unique_ptr<CActor[]> m_script;
    uint32_t m_size;
    uint32_t m_max;
    enum
    {
        SCRIPTNAME_MAX = 255,
        GROW_BY = 16,
        VERSION = 0,
        ALPHA = 0xff000000,
        BLACK = 0x000000 | ALPHA
    };
    inline void growArray();
};

#endif
