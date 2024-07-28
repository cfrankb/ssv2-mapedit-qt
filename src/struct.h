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
#ifndef __STRUCT_H
#define __STRUCT_H
#include <cstdint>

using scriptEntry_t = struct
{
    uint8_t attr;
    uint8_t type; // objType
    uint8_t u1;
    uint8_t u2;
    uint16_t imageId;
    uint8_t x;
    uint8_t y;
};

#endif