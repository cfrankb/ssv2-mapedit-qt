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
#ifndef __CACTOR_H
#define __CACTOR_H

#include <cstdint>

class CActor
{
public:
    CActor();
    ~CActor();

    enum
    {
        AIM_UP,
        AIM_DOWN,
        AIM_LEFT,
        AIM_RIGHT,
        BUTTON,
        TOTAL_AIMS = 4,
        AIM_NONE = 255
    };

    union
    {
        uint8_t attr;
        uint8_t task;
    };
    uint8_t type; // objType
    union
    {
        uint8_t u1;
        uint8_t aim;
        uint8_t triggerKey;
    };
    union
    {
        uint8_t u2;
        uint8_t changeTo;
        uint8_t seqOffset;
    };

    uint16_t imageId;
    uint8_t x;
    uint8_t y;

    void clear();
};

#endif
