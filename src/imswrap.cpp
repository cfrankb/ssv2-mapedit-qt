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
#include <cstdint>
#include <cctype>
#include <cstring>
#include <string>
#include "imswrap.h"
#include "defs.h"
#include "shared/FrameSet.h"
#include "shared/Frame.h"

#define _C(cl) \
    {          \
        cl, #cl}

#define _N(__X) #__X

using typeDef_t = struct
{
    int typeId;
    const char *typeName;
};

CImsWrap::CImsWrap()
{
    m_tiles = 0;
    m_imgCount = 0;
    m_entryCount = 0;
    m_stoEntries = 0;
    m_tileData = nullptr;
    m_imsLookup = nullptr;
    m_script = nullptr;
    m_stoData = nullptr;
    m_imsTable = nullptr;
    m_stoName = "";
    m_imsName = "";
    m_imsTableLenght = 0;
}

CImsWrap::~CImsWrap()
{
    freeIms();
    freeSto();
    freeScr();
}

// get image count from imsTable (static linked list)
int CImsWrap::getImageCount(uint8_t *imsTable)
{
    int i = 0;
    imsEntry_t *entry = reinterpret_cast<imsEntry_t *>(imsTable);
    do
    {
        ++i;
        entry = reinterpret_cast<imsEntry_t *>(imsTable + entry->offsetNext);
    } while (entry->offsetNext);
    return i;
}

// create an index from source linked list
void CImsWrap::createImsIndex(uint8_t *imsTable, imsEntry_t **imsIndex)
{
    int i = 0;
    imsEntry_t *entry = reinterpret_cast<imsEntry_t *>(imsTable);
    do
    {
        imsIndex[i++] = entry;
        entry = reinterpret_cast<imsEntry_t *>(imsTable + entry->offsetNext);
    } while (entry->offsetNext);
}

// create an index from source linked list
// and image name list
void CImsWrap::createImsIndex(uint8_t *imsTable, char *imsNames, imsLookup_t *imsIndex)
{
    int i = 0;
    imsEntry_t *ptrEntry = reinterpret_cast<imsEntry_t *>(imsTable);
    do
    {
        imsIndex[i++].ptrEntry = ptrEntry;
        ptrEntry = reinterpret_cast<imsEntry_t *>(imsTable + ptrEntry->offsetNext);
    } while (ptrEntry->offsetNext);

    i = 0;
    char *s = imsNames;
    while (s && *s)
    {
        char *next = strstr(s, "\r");
        if (next)
        {
            *next = 0;
            ++next;
        }
        imsIndex[i++].name = s;
        s = next;
    }
}

void CImsWrap::freeIms()
{
    if (m_tileData)
    {
        delete[] m_tileData;
    }
    m_tileData = nullptr;

    if (m_imsLookup)
    {
        delete[] m_imsLookup;
    }
    m_imsLookup = nullptr;

    if (m_imsTable)
    {
        delete[] m_imsTable;
    }
    m_imsTable = nullptr;

    m_imsName = "";
    m_tiles = 0;
    m_imgCount = 0;
    m_imsTableLenght = 0;
}

void CImsWrap::freeScr()
{
    if (m_script)
    {
        delete[] m_script;
    }
    m_script = nullptr;
    m_entryCount = 0;
}

void CImsWrap::freeSto()
{
    if (m_stoData)
    {
        delete[] m_stoData;
    }
    m_stoData = nullptr;
    m_stoEntries = 0;
}

bool CImsWrap::readSTO(const char *stoFilename)
{
    char imsName[IMSNAME_LENGTH];
    freeSto();

    FILE *sfileSTO = fopen(stoFilename, "rb");
    if (sfileSTO)
    {
        uint16_t dataLenght = 0;
        fread(&dataLenght, sizeof(dataLenght), 1, sfileSTO);
        fread(imsName, sizeof(imsName), 1, sfileSTO);
        m_stoEntries = dataLenght / sizeof(stoEntry_t);
        m_stoData = new stoEntry_t[m_stoEntries];
        fread(m_stoData, dataLenght, 1, sfileSTO);
        fclose(sfileSTO);

        printf("dataLenght: %d\n", dataLenght);
        printf("imsName: %s\n", imsName);
        for (int i = 0; i < m_stoEntries; ++i)
        {
            stoEntry_t &entry = m_stoData[i];
            printf("%d > task:0x%x class:0x%.2x (%s) imageId:%d\n",
                   i, entry.task,
                   entry.objtype, getTypeName(entry.objtype),
                   entry.imageID);
        }
        return true;
    }

    return false;
}

bool CImsWrap::readIMS(const char *imsFilename)
{
    freeIms();
    m_imsName = imsFilename;

    char imsNames[IMSNAME_MAX];
    memset(imsNames, 0, sizeof(imsNames));
    imsLookup_t *imsLookup = nullptr;
    FILE *imsFile = fopen(imsFilename, "rb");
    if (imsFile)
    {
        m_imsTableLenght = 0;
        m_tiles = 0;
        fread(&m_tiles, sizeof(uint16_t), 1, imsFile);
        fread(&m_imsTableLenght, sizeof(uint16_t), 1, imsFile);

        m_imsTable = new uint8_t[m_imsTableLenght];
        fread(m_imsTable, m_imsTableLenght - IMSHEADER_SIZE, 1, imsFile);
        m_imgCount = getImageCount(m_imsTable);
        m_imsLookup = new imsLookup_t[m_imgCount + 1];

        m_tileData = new fntEntry_t[m_tiles];
        fread(m_tileData, m_tiles * sizeof(fntEntry_t), 1, imsFile);
        fseek(imsFile, IMSHEADER_RESERVED, SEEK_CUR); // skip reserved
        fread(imsNames, IMSNAME_MAX, 1, imsFile);
        createImsIndex(m_imsTable, imsNames, m_imsLookup);
        fclose(imsFile);
        return true;
    }

    return false;
}

const char *CImsWrap::getImageName(int imageID)
{
    return imageID < m_imgCount ? m_imsLookup[imageID].name.c_str() : "UNKNOWN";
}

const char *CImsWrap::getTypeName(int typeId)
{
    const static typeDef_t typeDefs[]{
        _C(TYPE_BLANK),
        _C(TYPE_PLAYER),
        _C(TYPE_OXYGEN),
        _C(TYPE_TRANSPORTER),
        _C(TYPE_DIAMOND),
        _C(TYPE_FLOWER),
        _C(TYPE_FRUIT),
        _C(TYPE_MUSHROOM),
        _C(TYPE_MISC),
        _C(TYPE_DEADLYITEM),

        _C(TYPE_FISH),
        _C(TYPE_VCREA),
        _C(TYPE_VAMPIREPLANT),
        _C(TYPE_FLYPLAT),
        _C(TYPE_SPIDER),
        _C(TYPE_CANNIBAL),
        _C(TYPE_INMANGA),
        _C(TYPE_GREENFLEA),

        _C(TYPE_LADDER),
        _C(TYPE_TREE),
        _C(TYPE_BRIDGE),
        _C(TYPE_LADDERDING),

        _C(TYPE_SAND),
        _C(TYPE_TOPWATER),
        _C(TYPE_BOTTOMWATER),
        _C(TYPE_OBSTACLECLASS),
        _C(TYPE_STOPCLASS),
        _C(TYPE_LAVA),
    };

    const size_t maxTypes = sizeof(typeDefs) / sizeof(typeDef_t);
    for (size_t i = 0; i < maxTypes; ++i)
    {
        if (typeDefs[i].typeId == typeId)
        {
            return typeDefs[i].typeName;
        }
    }
    return "TYPE_UNKNOWN";
}

bool CImsWrap::readSCR(const char *scrFilename)
{
    uint16_t dataLenght = 0;
    char stoName[STONAME_LENGTH];
    freeScr();

    FILE *sfileSCR = fopen(scrFilename, "rb");
    if (sfileSCR)
    {
        fread(&dataLenght, sizeof(dataLenght), 1, sfileSCR);
        fread(&stoName, sizeof(stoName), 1, sfileSCR);
        m_stoName = stoName;
        m_entryCount = dataLenght / sizeof(scriptEntry_t);
        m_script = new scriptEntry_t[m_entryCount];
        fread(m_script, dataLenght, 1, sfileSCR);
        fclose(sfileSCR);
        return true;
    }
    return false;
}

const CImsWrap::rgba_t &CImsWrap::getPaletteColor(int i)
{
    // original color palette
    static constexpr uint32_t colors[]{
        0x00000000, 0xffab0303, 0xff03ab03, 0xffabab03, 0xff0303ab, 0xffab03ab, 0xff0357ab, 0xffababab,
        0xff575757, 0xffff5757, 0xff57ff57, 0xffffff57, 0xff5757ff, 0xffff57ff, 0xff57ffff, 0xffffffff,
        0xff000000, 0xff171717, 0xff232323, 0xff2f2f2f, 0xff3b3b3b, 0xff474747, 0xff535353, 0xff636363,
        0xff737373, 0xff838383, 0xff939393, 0xffa3a3a3, 0xffb7b7b7, 0xffcbcbcb, 0xffe3e3e3, 0xffffffff,
        0xffff0303, 0xffff0343, 0xffff037f, 0xffff03bf, 0xffff03ff, 0xffbf03ff, 0xff7f03ff, 0xff4303ff,
        0xff0303ff, 0xff0343ff, 0xff037fff, 0xff03bfff, 0xff03ffff, 0xff03ffbf, 0xff03ff7f, 0xff03ff43,
        0xff03ff03, 0xff43ff03, 0xff7fff03, 0xffbfff03, 0xffffff03, 0xffffbf03, 0xffff7f03, 0xffff4303,
        0xffff7f7f, 0xffff7f9f, 0xffff7fbf, 0xffff7fdf, 0xffff7fff, 0xffdf7fff, 0xffbf7fff, 0xff9f7fff,
        0xff7f7fff, 0xff7f9fff, 0xff7fbfff, 0xff7fdfff, 0xff7fffff, 0xff7fffdf, 0xff7fffbf, 0xff7fff9f,
        0xff7fff7f, 0xff9fff7f, 0xffbfff7f, 0xffdfff7f, 0xffffff7f, 0xffffdf7f, 0xffffbf7f, 0xffff9f7f,
        0xffffb7b7, 0xffffb7c7, 0xffffb7db, 0xffffb7eb, 0xffffb7ff, 0xffebb7ff, 0xffdbb7ff, 0xffc7b7ff,
        0xffb7b7ff, 0xffb7c7ff, 0xffb7dbff, 0xffb7ebff, 0xffb7ffff, 0xffb7ffeb, 0xffb7ffdb, 0xffb7ffc7,
        0xffb7ffb7, 0xffc7ffb7, 0xffdbffb7, 0xffebffb7, 0xffffffb7, 0xffffebb7, 0xffffdbb7, 0xffffc7b7,
        0xff730303, 0xff73031f, 0xff73033b, 0xff730357, 0xff730373, 0xff570373, 0xff3b0373, 0xff1f0373,
        0xff030373, 0xff031f73, 0xff033b73, 0xff035773, 0xff037373, 0xff037357, 0xff03733b, 0xff03731f,
        0xff037303, 0xff1f7303, 0xff3b7303, 0xff577303, 0xff737303, 0xff735703, 0xff733b03, 0xff731f03,
        0xff733b3b, 0xff733b47, 0xff733b57, 0xff733b63, 0xff733b73, 0xff633b73, 0xff573b73, 0xff473b73,
        0xff3b3b73, 0xff3b4773, 0xff3b5773, 0xff3b6373, 0xff3b7373, 0xff3b7363, 0xff3b7357, 0xff3b7347,
        0xff3b733b, 0xff47733b, 0xff57733b, 0xff63733b, 0xff73733b, 0xff73633b, 0xff73573b, 0xff73473b,
        0xff735353, 0xff73535b, 0xff735363, 0xff73536b, 0xff735373, 0xff6b5373, 0xff635373, 0xff5b5373,
        0xff535373, 0xff535b73, 0xff536373, 0xff536b73, 0xff537373, 0xff53736b, 0xff537363, 0xff53735b,
        0xff537353, 0xff5b7353, 0xff637353, 0xff6b7353, 0xff737353, 0xff736b53, 0xff736353, 0xff735b53,
        0xff430303, 0xff430313, 0xff430323, 0xff430333, 0xff430343, 0xff330343, 0xff230343, 0xff130343,
        0xff030343, 0xff031343, 0xff032343, 0xff033343, 0xff034343, 0xff034333, 0xff034323, 0xff034313,
        0xff034303, 0xff134303, 0xff234303, 0xff334303, 0xff434303, 0xff433303, 0xff432303, 0xff431303,
        0xff432323, 0xff43232b, 0xff432333, 0xff43233b, 0xff432343, 0xff3b2343, 0xff332343, 0xff2b2343,
        0xff232343, 0xff232b43, 0xff233343, 0xff233b43, 0xff234343, 0xff23433b, 0xff234333, 0xff23432b,
        0xff234323, 0xff2b4323, 0xff334323, 0xff3b4323, 0xff434323, 0xff433b23, 0xff433323, 0xff432b23,
        0xff432f2f, 0xff432f33, 0xff432f37, 0xff432f3f, 0xff432f43, 0xff3f2f43, 0xff372f43, 0xff332f43,
        0xff2f2f43, 0xff2f3343, 0xff2f3743, 0xff2f3f43, 0xff2f4343, 0xff2f433f, 0xff2f4337, 0xff2f4333,
        0xff2f432f, 0xff33432f, 0xff37432f, 0xff3f432f, 0xff43432f, 0xff433f2f, 0xff43372f, 0xff43332f,
        0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000, 0xff000000};

    return reinterpret_cast<const rgba_t *>(colors)[i];
}

const char *CImsWrap::stoName()
{
    return m_stoName.c_str();
}

void CImsWrap::debug(const char *filename)
{
    FILE *tfile = fopen(filename, "wb");
    if (tfile)
    {
        fprintf(tfile, "imsfilename: %s\n", m_imsName.c_str());
        fprintf(tfile, "tiles: %d\n", m_tiles);
        fprintf(tfile, "imsLenght: %d\n", m_imsTableLenght);
        fprintf(tfile, "imsCount: %d\n\n", m_imgCount);

        for (int i = 0; i < m_imgCount; ++i)
        {
            imsLookup_t &item = m_imsLookup[i];
            fprintf(tfile, "%d len=%d hei=%d %s\n", i, item.ptrEntry->len, item.ptrEntry->hei, item.name.c_str());
        }

        fprintf(tfile, "\nstoName: %s\n", m_stoName.c_str());
        fprintf(tfile, "    m_entryCount: %d\n", m_entryCount);
        fprintf(tfile, "    m_entryCount * size: %lu\n\n", m_entryCount * sizeof(scriptEntry_t));

        for (int i = 0; i < m_entryCount; ++i)
        {
            scriptEntry_t &entry = m_script[i];
            fprintf(tfile, "#%d attr %x type %.2x (%s)\n", i, entry.attr, entry.type, getTypeName(entry.type));
            fprintf(tfile, "    u1 %x u2 %x imageId %d (%s)\n", entry.u1, entry.u2, entry.imageId, getImageName(entry.imageId));
            fprintf(tfile, "    x:%d y:%d \n\n", entry.x, entry.y);
        }
    }
}

void CImsWrap::toFrameSet(CFrameSet &frameSet, FILE *mapFile)
{
    frameSet.forget();
    for (int i = 0; i < m_imgCount; ++i)
    {
        auto &lookup = m_imsLookup[i];
        if (lookup.name[0] == '+')
        {
            break;
        }
        if (mapFile)
        {
            fprintf(mapFile, "%.4x %s\n", i, lookup.name.c_str());
        }

        const auto &entry = lookup.ptrEntry;
        uint16_t *fntData = &entry->fntData;
        CFrame *frame = new CFrame(FNT_BLOCK_SIZE * entry->len, FNT_BLOCK_SIZE * entry->hei);
        for (int y = 0; y < entry->hei; ++y)
        {
            for (int x = 0; x < entry->len; ++x)
            {
                const auto fntBlock = *fntData;
                auto &tile = m_tileData[fntBlock];
                auto pixels = tile.pixels;
                for (int yy = 0; yy < FNT_BLOCK_SIZE; ++yy)
                {
                    for (int xx = 0; xx < FNT_BLOCK_SIZE; ++xx)
                    {
                        auto &rgba = frame->at(x * FNT_BLOCK_SIZE + xx, y * FNT_BLOCK_SIZE + yy);
                        const rgba_t &color = getPaletteColor(*pixels++);
                        rgba = *(reinterpret_cast<const uint32_t *>(&color));
                    }
                }
                ++fntData;
            }
        }
        frameSet.add(frame);
    }
}

void CImsWrap::drawScreen(CFrame &screen, CFrameSet &frameSet)
{
    screen.fill(BLACK);
    for (int i = 0; i < m_entryCount; ++i)
    {
        const auto &entry = m_script[i];
        int x = entry.x * FNT_BLOCK_SIZE;
        int y = entry.y * FNT_BLOCK_SIZE;
        if (entry.imageId >= frameSet.getSize())
        {
            printf("imageID out of bound: %d [%s]\n", entry.imageId, getTypeName(entry.type));
            continue;
        }
        CFrame *frame = frameSet[entry.imageId];
        for (int yy = 0; yy < frame->hei(); ++yy)
        {
            if (yy + y >= screen.hei())
            {
                break;
            }
            uint32_t *rgba = &screen.at(x, y + yy);
            for (int xx = 0; xx < frame->len(); ++xx)
            {
                if (xx + x >= screen.len())
                {
                    break;
                }
                const uint32_t &pixel = frame->at(xx, yy);
                if (pixel)
                {
                    rgba[xx] = pixel;
                }
            }
        }
    }
}

const CImsWrap::stoEntry_t *CImsWrap::stoData(int &count)
{
    count = m_stoEntries;
    return m_stoData;
}

const char *CImsWrap::taskName(int i)
{
    static const char *tasks[]{
        _N(TASK_NONE),
        _N(TASK_REMOVE),
        _N(TASK_SOURCE),
        _N(TASK_DEST),
        _N(TASK_CHANGE),
        _N(TASK_MESSAGER),
    };
    return tasks[i];
}

void CImsWrap::toImageList(std::vector<std::string> &list)
{
    list.clear();
    for (int i = 0; i < m_imgCount; ++i)
    {
        auto &lookup = m_imsLookup[i];
        if (lookup.name[0] == '+')
        {
            break;
        }
        list.emplace_back(lookup.name);
    }
}