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
#include <cstring>
#include "script.h"
#include "defs.h"
#include "actor.h"

CScript::CScript()
{
    m_script = nullptr;
    m_size = 0;
    m_max = m_size;
    m_rgbaColor = BLACK;
}

CScript::CScript(std::unique_ptr<CActor[]> &script, uint32_t size)
{
    // TODO: check this
    m_script = std::move(script);
    m_size = size;
    m_max = m_size;
    m_rgbaColor = BLACK;
}

CScript::~CScript()
{
    forget();
}

void CScript::forget()
{
    if (m_script && m_script.get())
    {
        auto ptr = m_script.release(); // pointer to no-longer-managed object
        delete[] ptr;
    }
    m_script = nullptr;
    m_size = 0;
    m_max = 0;
}

bool CScript::write(FILE *tfile)
{
    // write entry count
    fwrite(&m_size, sizeof(uint16_t), 1, tfile);
    uint16_t version = VERSION;
    fwrite(&version, sizeof(uint16_t), 1, tfile);
    fwrite(&m_rgbaColor, sizeof(m_rgbaColor), 1, tfile);

    // save tileset name
    char tileset[TILESET_NAME_MAX];
    memset(tileset, 0, sizeof(tileset));
    strncpy(tileset, m_tileset.c_str(), sizeof(tileset));
    fwrite(tileset, sizeof(tileset), 1, tfile);

    // write script
    fwrite(m_script.get(), sizeof(CActor) * m_size, 1, tfile);

    // write scriptname + padding
    char name[SCRIPTNAME_MAX];
    memset(name, 0, sizeof(name));
    strncpy(name, m_name.c_str(), sizeof(name) - 1);
    auto size = strlen(name);
    //    printf("name: %s %d 0x%.2x\n", name, size + 1, size + 1);
    fwrite(&size, sizeof(uint8_t), 1, tfile);
    fwrite(name, size, 1, tfile);
    auto padding = ((4 - (1 + size)) & 3) & 3;
    auto tmp = 0;
    //  printf("padding 0x%.2x\n", padding);
    fwrite(&tmp, padding, 1, tfile);
    return true;
}

bool CScript::read(FILE *sfile)
{
    forget();

    // read entry count
    m_size = 0;
    uint16_t version = 0;
    fread(&m_size, sizeof(uint16_t), 1, sfile);
    fread(&version, sizeof(uint16_t), 1, sfile);
    fread(&m_rgbaColor, sizeof(m_rgbaColor), 1, sfile);
    if (version != VERSION)
    {
        return false;
    }
    m_max = m_size;

    // read tileset name
    char tileset[TILESET_NAME_MAX + 1]{};
    memset(tileset, 0, sizeof(tileset));
    fread(tileset, TILESET_NAME_MAX, 1, sfile);
    m_tileset = tileset;

    // read script
    m_script = std::make_unique<CActor[]>(m_size);
    fread(m_script.get(), sizeof(CActor) * m_size, 1, sfile);

    // read scriptname
    char name[SCRIPTNAME_MAX + 1]{};
    memset(name, 0, sizeof(name));
    uint8_t size = 0;
    fread(&size, sizeof(uint8_t), 1, sfile);
    fread(name, size, 1, sfile);
    m_name = name;
    return true;
}

bool CScript::fromMemory(const uint8_t *data)
{
    forget();

    // read entry count
    uint8_t *p = const_cast<uint8_t *>(data);
    auto memread = [&p](auto __m__, auto __u__)
    {
        memcpy(p, __m__, __u__);
        p += __u__;
    };
    uint16_t version = 0;
    m_size = 0;
    memread(&m_size, sizeof(uint16_t));
    memread(&version, sizeof(uint16_t));
    memread(&m_rgbaColor, sizeof(m_rgbaColor));
    m_max = m_size;
    if (version != VERSION)
    {
        return false;
    }

    // read tileset name
    char tileset[TILESET_NAME_MAX + 1]{};
    memset(tileset, 0, sizeof(tileset));
    memread(tileset, TILESET_NAME_MAX);
    m_tileset = tileset;

    // read script
    m_script = std::make_unique<CActor[]>(m_size);
    memread(m_script.get(), sizeof(CActor) * m_size);

    // read scriptname
    char name[SCRIPTNAME_MAX + 1]{};
    memset(name, 0, sizeof(name));
    uint8_t size = 0;
    memread(&size, sizeof(uint8_t));
    memread(name, size);
    m_name = name;
    return true;
}

std::string CScript::name() const
{
    return m_name;
}

void CScript::setName(const std::string &name)
{
    m_name = name;
}

std::string CScript::tileset() const
{
    return m_tileset;
}

void CScript::setTileSet(const std::string &tileset)
{
    m_tileset = tileset;
}

void CScript::growArray()
{
    if (m_size == m_max)
    {
        m_max += GROW_BY;
        std::unique_ptr<CActor[]> tmp = std::make_unique<CActor[]>(m_max);
        CActor *t = tmp.get();
        CActor *s = m_script.get();
        for (uint32_t i = 0; i < m_size; ++i)
        {
            t[i] = s[i];
        }
        m_script.swap(tmp);
    }
}

int CScript::add(const CActor &entry)
{
    growArray();
    m_script.get()[m_size] = entry;
    return m_size++;
}

int CScript::insertAt(uint32_t i, const CActor &entry)
{
    growArray();
    CActor *s = m_script.get();
    for (uint32_t j = m_size; j > i; --j)
    {
        s[j] = s[j - 1];
    }
    m_script.get()[i] = entry;
    ++m_size;
    return i;
}

void CScript::removeAt(int i)
{
    CActor *s = m_script.get();
    for (uint32_t j = i; j < m_size - 1; ++j)
    {
        s[j] = s[j + 1];
    }
    --m_size;
}

int CScript::findPlayerIndex() const
{
    CActor *s = m_script.get();
    for (uint32_t i = 0; i < m_size; ++i)
    {
        if (s[i].type == TYPE_PLAYER)
        {
            return i;
        }
    }
    return NOT_FOUND;
}

int CScript::countType(uint8_t type) const
{
    int count = 0;
    CActor *s = m_script.get();
    for (uint32_t i = 0; i < m_size; ++i)
    {
        if (s[i].type == type)
        {
            ++count;
        }
    }
    return count;
}

void CScript::sort()
{
    std::unique_ptr<CActor[]> tmp = std::make_unique<CActor[]>(m_size);
    int j = 0;
    CActor *s = m_script.get();
    CActor *t = tmp.get();
    for (uint32_t i = 0; i < m_size; ++i)
    {
        const CActor &entry{s[i]};
        if (CScript::isBackgroundType(entry.type))
        {
            t[j++] = entry;
        }
    }
    for (uint32_t i = 0; i < m_size; ++i)
    {
        const CActor &entry{s[i]};
        if (!CScript::isBackgroundType(entry.type))
        {
            t[j++] = entry;
        }
    }
    m_script.swap(tmp);
}

void CScript::shift(int aim)
{
    CActor *s = m_script.get();
    for (uint32_t i = 0; i < m_size; ++i)
    {
        CActor &entry{s[i]};
        switch (aim)
        {
        case UP:
            --entry.y;
            break;
        case DOWN:
            ++entry.y;
            break;
        case LEFT:
            --entry.x;
            break;
        case RIGHT:
            ++entry.x;
        }
    }
}

uint32_t CScript::rgbaColor()
{
    return m_rgbaColor;
}

void CScript::setRgbaColor(uint32_t color)
{
    m_rgbaColor = color;
}