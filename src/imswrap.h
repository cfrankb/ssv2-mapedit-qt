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
#ifndef __IMSWRAP_H
#define __IMSWRAP_H

#include <cstdint>
#include <string>
#include <vector>

class CFrameSet;
class CFrame;

class CImsWrap
{
public:
    CImsWrap();
    ~CImsWrap();

    using stoEntry_t = struct
    {
        uint8_t task;
        uint8_t objtype;
        uint16_t imageID;
    };

    bool readIMS(const char *imsFilename);
    bool readSCR(const char *scrFilename);
    bool readSTO(const char *stoFilename);
    void toFrameSet(CFrameSet &frameSet, FILE *mapFile);
    void drawScreen(CFrame &screen, CFrameSet &frameSet);
    void debug(const char *filename);
    const char *stoName();
    static const char *getTypeName(int typeId);
    const stoEntry_t *stoData(int &count);
    static const char *taskName(int i);
    void toImageList(std::vector<std::string> &list);

protected:
    using fntEntry_t = struct
    {
        uint8_t pixels[64];
    };

    using imsEntry_t = struct
    {
        uint16_t offsetNext;
        uint16_t len;
        uint16_t hei;
        uint16_t fntData;
    };

    using imsLookup_t = struct
    {
        imsEntry_t *ptrEntry;
        std::string name;
    };

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

    using rgba_t = struct
    {
        uint8_t red;
        uint8_t green;
        uint8_t blue;
        uint8_t alpha;
    };

    enum
    {
        IMSNAME_MAX = 8192,
        FNT_BLOCK_SIZE = 8,
        IMSHEADER_SIZE = 4,
        IMSHEADER_RESERVED = 2,
        STONAME_LENGTH = 16,
        IMSNAME_LENGTH = 16,
        BLACK = 0xff000000,
    };

    int getImageCount(uint8_t *imsTable);
    void createImsIndex(uint8_t *imsTable, imsEntry_t **imsIndex);
    void createImsIndex(uint8_t *imsTable, char *imsNames, imsLookup_t *imsIndex);
    void freeIms();
    void freeScr();
    void freeSto();
    inline const char *getImageName(int imageID);
    inline const rgba_t &getPaletteColor(int i);

    std::string m_stoName;
    std::string m_imsName;
    int m_tiles;
    int m_imgCount;
    int m_imsTableLenght;
    int m_entryCount;
    int m_stoEntries;
    fntEntry_t *m_tileData;
    imsLookup_t *m_imsLookup;
    scriptEntry_t *m_script;
    stoEntry_t *m_stoData;
    uint8_t *m_imsTable;
};

#endif