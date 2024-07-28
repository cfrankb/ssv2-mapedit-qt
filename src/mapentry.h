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
#ifndef __MAPENTRY_H
#define __MAPENTRY_H
#include "mapentry.h"
#include <cstdint>

class CMapEntry
{

private:
    uint8_t m_bkType;
    uint8_t m_player;
    uint16_t m_acEntry;
    uint16_t m_fwEntry[2];

public:
    enum
    {
        fwCount = sizeof(m_fwEntry) / sizeof(decltype(*m_fwEntry))
    };

    uint16_t fwEntry(int i) const
    {
        return m_fwEntry[i];
    }
    CMapEntry();
    ~CMapEntry();

    constexpr inline uint8_t bk() const
    {
        return m_bkType;
    }

    constexpr inline void setBk(uint8_t bkv)
    {
        m_bkType = bkv;
    }

    constexpr inline bool player() const
    {
        return m_player;
    }

    constexpr inline void setPlayer(bool p)
    {
        m_player = static_cast<uint8_t>(p);
    }

    constexpr inline bool isEmpty() const
    {
        for (uint32_t i = 0; i < fwCount; ++i)
        {
            if (m_fwEntry[i] != 0)
            {
                return false;
            }
        }
        return (m_bkType == 0) &&
               (m_acEntry == 0) &&
               (m_player == 0);
    }

    constexpr inline uint16_t acEntry() const
    {
        return m_acEntry;
    }

    constexpr inline void setAcEntry(const uint16_t ac)
    {
        m_acEntry = ac;
    }

    constexpr inline void setFwEntry(const uint16_t fw)
    {
        for (uint32_t i = 0; i < fwCount; ++i)
        {
            if (m_fwEntry[i] == fw)
            {
                return;
            }
        }
        for (uint32_t i = 0; i < fwCount; ++i)
        {
            if (m_fwEntry[i] == 0)
            {
                m_fwEntry[i] = fw;
                return;
            }
        }
    }

    constexpr inline void removeFwEntry(const uint16_t fw)
    {
        for (uint32_t i = 0; i < fwCount; ++i)
        {
            if (m_fwEntry[i] == fw)
            {
                m_fwEntry[i] = 0;
                return;
            }
        }
    }

    void debug() const;
};
#endif