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
#include <memory>

#include "shared/FrameSet.h"
#include "shared/Frame.h"
#include "shared/FileWrap.h"
#include "shared/qtgui/qfilewrap.h"
#include "script.h"
#include "scriptarch.h"
#include "struct.h"
#include "framemap.h"
#include "imswrap.h"

using filedef_t = struct
{
    const char *imsFile;
    const char *scrFile;
    const char *name;
};

constexpr filedef_t filedefs[] = {
    //   {"intro.ims", "_lev00.scr", "il est temps de partir."},
    {"jungle.ims", "_lev01.scr", "et voila que commence ton aventure_"},
    {"jungle.ims", "_lev02.scr", "attention de ne pas tomber!"},
    {"jungle.ims", "_lev03.scr", "les fruits du diable_"},
    {"jungle.ims", "_lev04.scr", "la mort te guete."},
    {"jungle.ims", "_lev05.scr", "mechant! mechant!"},
    {"jungle.ims", "_lev06.scr", "jargons divers!"},
    {"jungle.ims", "_lev07.scr", "arachides fumees!"},
    {"jungle.ims", "_lev08.scr", "le nouveau_"},
    {"ocean.ims", "_lev09.scr", "la transition_"},
    {"ocean.ims", "_lev10.scr", "au milieu de la mer_"},
    {"ocean.ims", "_lev11.scr", "bla bla bla!"},
    {"ocean.ims", "_lev12.scr", "solution salee_"},
    {"ocean.ims", "_lev13.scr", "***solution salee_"},
    {"caves.ims", "_lev20.scr", "sentier mortel."},
    {"caves.ims", "_lev21.scr", "le grand gouffre_"},
    {"caves.ims", "_lev22.scr", "***le grand gouffre_"},
    {"oldvla.ims", "_leva0.scr", "tu vas reconnaitre!"},
    {"oldvla.ims", "_leva3.scr", "raison passion."},
    {"oldvla.ims", "_leva2.scr", "amionoux truc"},
    {"oldvla.ims", "_leva1.scr", "le grand complexe_"},
    {"oldvla.ims", "_leva4.scr", "zoomy zoom zoom!"},
    {"oldvla.ims", "_leva5.scr", "missing level"},
    {"oldvla.ims", "_leva6.scr", "whoops!"},
    {"oldvla.ims", "_leva7.scr", "***whoops!"},
};

void exportIms()
{
    CImsWrap ims;
    CFileWrap file;
    CFrameSet frameSet;
    std::string tilesets[] = {
        "jungle",
        "ocean",
        "caves",
        "oldvla"};

    for (int i = 0; i < 4; ++i)
    {
        std::string oblFile = "out/" + tilesets[i] + ".obl";
        std::string imsFile = "data/" + tilesets[i] + ".ims";
        std::string mapFile = "out/" + tilesets[i] + ".map";
        if (file.open(oblFile.c_str(), "wb"))
        {
            ims.readIMS(imsFile.c_str());
            FILE *tfileMap = fopen(mapFile.c_str(), "wb");
            ims.toFrameSet(frameSet, tfileMap);
            frameSet.write(file);
            file.close();
            fclose(tfileMap);
            printf("%s -> %d images\n", oblFile.c_str(), frameSet.getSize());
        }
    }
}

void debugScr()
{
    CImsWrap ims;
    constexpr size_t count = sizeof(filedefs) / sizeof(filedef_t);
    for (size_t i = 0; i < count; ++i)
    {
        const filedef_t &def = filedefs[i];
        const std::string scrName = std::string("data/") + def.scrFile;
        ims.readSCR(scrName.c_str());
        printf("%s %s %s\n", def.scrFile, def.imsFile, ims.stoName());
    }
}

bool compositeLevel(std::string imsFile, std::string scrFile, std::string outFile)
{
    CImsWrap ims;
    if (!ims.readIMS(imsFile.c_str()))
    {
        printf("readims %s failed\n", imsFile.c_str());
        return true;
    }
    if (!ims.readSCR(scrFile.c_str()))
    {
        printf("readscr %s failed\n", scrFile.c_str());
        return true;
    }
    CFrameSet frameSet;
    // CFrame screen = CFrame(320, 240);
    CFrame screen = CFrame(1024, 1024);
    ims.toFrameSet(frameSet, nullptr);
    ims.drawScreen(screen, frameSet);
    CFileWrap file;
    if (file.open(outFile.c_str(), "wb"))
    {
        uint8_t *png = nullptr;
        int size;
        screen.toPng(png, size);
        file.write(png, size);
        file.close();
        delete[] png;
    }
    return true;
}

void compositeAll()
{
    constexpr size_t count = sizeof(filedefs) / sizeof(filedef_t);
    for (size_t i = 0; i < count; ++i)
    {
        const filedef_t &def = filedefs[i];
        const std::string scrFile = std::string("techdocs/data/old/") + def.scrFile;
        const std::string imsFile = std::string("data/") + def.imsFile;
        char pngName[16];
        strncpy(pngName, def.scrFile, sizeof(pngName));
        char *p = strstr(pngName, ".");
        if (p)
        {
            strcpy(p, ".png");
        }
        else
        {
            strcat(pngName, ".png");
        }
        const std::string outFile = std::string("out/") + pngName;
        compositeLevel(imsFile.c_str(), scrFile.c_str(), outFile.c_str());
    }
}

bool createScriptArch()
{
    CScriptArch arch;
    CImsWrap ims;
    constexpr size_t count = sizeof(filedefs) / sizeof(filedef_t);
    for (size_t i = 0; i < count; ++i)
    {
        const filedef_t &def = filedefs[i];
        const std::string scrFile = std::string("techdocs/data/old/") + def.scrFile;
        printf("file:%s\n", scrFile.c_str());
        FILE *sfileSCR = fopen(scrFile.c_str(), "rb");
        if (sfileSCR)
        {
            uint16_t dataLenght;
            fread(&dataLenght, 2, 1, sfileSCR);
            fseek(sfileSCR, 16, SEEK_CUR);
            auto entryCount = dataLenght / sizeof(scriptEntry_t);
            std::unique_ptr<CActor[]> scriptData = std::make_unique<CActor[]>(entryCount);
            fread(scriptData.get(), dataLenght, 1, sfileSCR);
            fclose(sfileSCR);
            CScript *script = new CScript(scriptData, entryCount);
            std::string tileset = def.imsFile;
            auto j = tileset.find(".");
            if (j != std::string::npos)
            {
                tileset.resize(j);
            }
            tileset.resize(8);
            script->setName(def.name);
            script->setTileSet(tileset);
            arch.add(script);
        }
        else
        {
            printf("readscr %s failed\n", scrFile.c_str());
            return false;
        }
    }

    const char *archfile = "out/levels.scrx";
    printf("writing archfile: %s\n", archfile);
    if (!arch.write(archfile))
    {
        printf("failed to create: %s\n", archfile);
        return false;
    }
    return true;
}

bool testArch()
{
    CScriptArch arch;
    const char *archfileS = "out/levels.scrx";
    const char *archfileT = "out/levels1.scrx";
    printf("reading archfile: %s\n", archfileS);
    if (!arch.read(archfileS))
    {
        printf("failed to read: %s\n", archfileS);
        return false;
    }
    printf("writing archfile: %s\n", archfileT);
    if (!arch.write(archfileT))
    {
        printf("failed to create: %s\n", archfileT);
        return false;
    }
    return true;
}

void generateSTX()
{
    const char *tilesets[] = {
        "jungle",
        "caves",
        "ocean",
        "oldvla",
    };

    for (uint32_t i = 0; i < sizeof(tilesets) / sizeof(tilesets[0]); ++i)
    {
        const std::string stoFile = std::string("techdocs/data/old/") + tilesets[i] + std::string(".sto");
        const std::string imsFile = std::string("data/") + tilesets[i] + std::string(".ims");
        const std::string stxFile = std::string("out/") + tilesets[i] + std::string(".stx");
        CImsWrap wrap;
        if (!wrap.readIMS(imsFile.c_str()))
        {
            printf("failed to read: %s\n", imsFile.c_str());
            continue;
        }
        if (!wrap.readSTO(stoFile.c_str()))
        {
            printf("failed to read: %s\n", stoFile.c_str());
            continue;
        }

        std::vector<std::string> imageList;

        int count;
        const CImsWrap::stoEntry_t *entries = wrap.stoData(count);
        wrap.toImageList(imageList);

        char tmp[1024];
        CFileWrap file;
        if (file.open(stxFile.c_str(), "wb"))
        {
            file += "[images]\n";
            for (size_t i = 0; i < imageList.size(); ++i)
            {
                sprintf(tmp, "%.4x %s\n", static_cast<uint32_t>(i), imageList[i].c_str());
                char *p = strstr(tmp, "#");
                while (p)
                {
                    *p = '_';
                    p = strstr(tmp, "#");
                }
                file += tmp;
            }
            file += "\n[types]\n";
            for (int i = 0; i < count; ++i)
            {
                auto &cur = entries[i];
                sprintf(tmp, "%.2x %.2x %.4x # %-16s %-20s %s\n",
                        cur.task, cur.objtype, cur.imageID,
                        CImsWrap::taskName(cur.task),
                        CImsWrap::getTypeName(cur.objtype),
                        cur.imageID < imageList.size() ? imageList[cur.imageID].c_str() : "???");
                file += tmp;
            }
            file.close();
        }
    }
}


void debugLevel(const char *filename,  CScript *script)
{
    std::unordered_map<uint16_t, std::string> imageNames;
    std::string mapFile = std::string(":/data/") + script->tileset() + ".stx";
    printf("reading: %s\n", mapFile.c_str());

    QFileWrap file;
    if (file.open(mapFile.c_str(), "rb")) {
        size_t size = file.getSize();
        char *data = new char[size + 1];
        data[size] = 0;
        file.read(data, size);
        file.close();
        char *p = strstr(data, "\n");
        p = p ? ++p: nullptr;

        uint16_t i = 0;
        while (p && p[0]!='[' && *p)
        {
            char *e = strstr(p, "\n");
            if (e)
            {
                *e = 0;
            }
            if (*p)
            {
                imageNames[i] = p;
            }
            ++i;
            p = e ? ++e : nullptr;
        }
        delete[] data;
    }

    printf("total images:%zu\n", imageNames.size());

    FILE *tfile = fopen(filename, "wb");
    if (tfile)
    {
        fprintf(tfile, "imsfilename: %s\n", script->tileset().c_str());
        //        fprintf(tfile, "tiles: %d\n\n", frameSet->getSize());
        for (int i = 0; i < script->getSize(); ++i)
        {
            const CActor &entry = (*script)[i];
            fprintf(tfile, "#%d attr %x type %.2x (%s)\n", i, entry.attr, entry.type, CImsWrap::getTypeName(entry.type));
            fprintf(tfile, "    u1 %x u2 %x imageId %d (%s)\n", entry.u1, entry.u2, entry.imageId, imageNames[entry.imageId].c_str());
            fprintf(tfile, "    x:%d y:%d \n\n", entry.x, entry.y);
        }
    }
}

void debugFrameMap(const char *outFile, CFrameMap *frameMap, CFrameSet *frameSet, uint8_t *fontData)
{
    const int FNT_BLOCK_SIZE = 8;
    CFrameSet fs;
    for (int i = 0; i < frameSet->getSize(); ++i)
    {
        CFrame *frame = new CFrame((*frameSet)[i]);
        uint8_t *map = frameMap->mapPtr(i);
        for (int j = 0; j < frame->hei() / FNT_BLOCK_SIZE; ++j)
        {
            for (int k = 0; k < frame->len() / FNT_BLOCK_SIZE; ++k)
            {
                uint8_t c = *map++;
                uint8_t *p = &fontData[c * FNT_BLOCK_SIZE];
                for (int y = 0; y < FNT_BLOCK_SIZE; ++y)
                {
                    uint8_t bits = p[y];
                    for (int x = 0; x < FNT_BLOCK_SIZE; ++x)
                    {
                        if (bits & 1)
                        {
                            frame->at(k * FNT_BLOCK_SIZE + x, j * FNT_BLOCK_SIZE + y) = 0xff00ffff;
                        }
                        bits = bits >> 1;
                    }
                }
            }
        }
        fs.add(frame);
    }

    CFileWrap file;
    if (file.open(outFile, "wb"))
    {
        fs.write(file);
        file.close();
    }
}
