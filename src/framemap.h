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
#ifndef __FRAMEMAP_H
#define __FRAMEMAP_H

#include <cstdint>
#include <unordered_set>
class CFrameSet;
class CFrame;

class CFrameMap
{
public:
    CFrameMap();
    ~CFrameMap();

    bool write(const char *filename);
    bool read(const char *filename);
    void fromFrameSet(CFrameSet &frameSet, std::unordered_set<uint16_t> &xmap);
    uint8_t *mapPtr(int i);
    uint8_t *operator[](int i);

private:
    enum
    {
        FNT_BLOCK_SIZE = 8,
        THRESHOLD = 16,
    };
    uint8_t **m_mapIndex;
    uint8_t *m_mapData;
    int m_dataSize;
    int scoreFromTile(CFrame &frame, int baseX, int baseY);
    void forget();
};

#endif
