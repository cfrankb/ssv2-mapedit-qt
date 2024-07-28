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
#include <cstdio>
#include <unordered_set>
#include "framemap.h"
#include "shared/FrameSet.h"
#include "shared/Frame.h"

CFrameMap::CFrameMap()
{
    m_mapIndex = nullptr;
    m_mapData = nullptr;
    m_dataSize = 0;
}

CFrameMap::~CFrameMap()
{
    forget();
}

void CFrameMap::forget()
{
    m_dataSize = 0;
    if (m_mapIndex)
    {
        delete[] m_mapIndex;
        m_mapIndex = nullptr;
    }
    if (m_mapData)
    {
        delete[] m_mapData;
        m_mapData = nullptr;
    }
}

void CFrameMap::fromFrameSet(CFrameSet &frameSet, std::unordered_set<uint16_t> &xmap)
{
    forget();

    int count = frameSet.getSize();
    m_mapIndex = new uint8_t *[count];
    m_dataSize = 0;
    for (int i = 0; i < count; ++i)
    {
        CFrame &frame = *frameSet[i];
        m_dataSize += frame.len() / FNT_BLOCK_SIZE * frame.hei() / FNT_BLOCK_SIZE;
    }

    m_mapData = new uint8_t[m_dataSize];
    uint8_t *p = m_mapData;

    for (int i = 0; i < count; ++i)
    {
        CFrame &frame = *frameSet[i];
        m_mapIndex[i] = p;
        for (int y = 0; y < frame.hei() / FNT_BLOCK_SIZE; ++y)
        {
            for (int x = 0; x < frame.len() / FNT_BLOCK_SIZE; ++x)
            {
                const uint8_t score = xmap.count(i) == 0
                                          ? scoreFromTile(frame, x * FNT_BLOCK_SIZE, y * FNT_BLOCK_SIZE)
                                          : 0xff;
                *p++ = score >= THRESHOLD ? score : 0;
            }
        }
    }
}

int CFrameMap::scoreFromTile(CFrame &frame, int baseX, int baseY)
{
    int score = 0;
    for (int y = 0; y < FNT_BLOCK_SIZE; ++y)
    {
        for (int x = 0; x < FNT_BLOCK_SIZE; ++x)
        {
            uint32_t &pixel = frame.at(baseX + x, baseY + y);
            score += (pixel >> 24) & 1;
        }
    }
    return score;
}

uint8_t *CFrameMap::mapPtr(int i)
{
    return m_mapIndex[i];
}

bool CFrameMap::write(const char *filename)
{
    // TODO: fix this
    FILE *tfile = fopen(filename, "wb");
    if (tfile)
    {
        fwrite("FMAP", 4, 1, tfile);
        fwrite(&m_dataSize, sizeof(m_dataSize), 1, tfile);
        fwrite(m_mapData, m_dataSize, 1, tfile);
        fclose(tfile);
    }
    return tfile != nullptr;
}

bool CFrameMap::read(const char *filename)
{
    // TODO: implement this
    return true;
}
uint8_t *CFrameMap::operator[](int i)
{
    return mapPtr(i);
}