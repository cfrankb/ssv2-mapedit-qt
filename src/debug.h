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
#ifndef __DEBUG__H
#define __DEBUG__H

class CScript;
class CFrameSet;
class CFrameMap;
#include <cstdint>

bool testArch();
bool createScriptArch();
void exportIms();
void compositeAll();
void generateSTX();
void debugLevel(const char *filename, CScript *script);
void debugFrameMap(const char *outFile, CFrameMap *frameMap, CFrameSet *frameSet, uint8_t *fontData);

#endif
