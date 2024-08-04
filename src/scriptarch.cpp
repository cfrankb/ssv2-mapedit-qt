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
#include "scriptarch.h"
#include "script.h"
#include <cstdio>
#include <cstring>
#include <string>

static constexpr const char DEFAULT_GAMEID[] = {'?', '?', '?', '?'};

CScriptArch::CScriptArch()
{
    m_size = 0;
    m_max = GROW_BY;
    m_scripts = std::make_unique<CScript *[]>(m_max);
    memcpy(m_gameID, DEFAULT_GAMEID, sizeof(DEFAULT_GAMEID));
}

CScriptArch::~CScriptArch()
{
    forget();
}

bool CScriptArch::read(const char *filename)
{
    /*
    4 signature      "SCRX"
    2 version
    2 script count
    4 index offset
    */
    FILE *sfile = fopen(filename, "rb");
    if (sfile)
    {
        forget();
        char signature[SIGNATURE_SIZE]{0, 0, 0, 0};
        uint16_t version;
        uint16_t count;
        uint32_t indexPtr;
        fread(signature, SIGNATURE_SIZE, 1, sfile);
        fread(&version, sizeof(version), 1, sfile);
        if (memcmp(signature, SIGNATURE, sizeof(signature)) != 0)
        {
            m_lastError = "invalid signature";
            printf("invalid signature: %s\n", signature);
            fclose(sfile);
            return false;
        }
        if (version > VERSION)
        {
            m_lastError = "invalid version";
            printf("invalid version: %d\n", version);
            fclose(sfile);
            return false;
        }
        fread(&count, sizeof(count), 1, sfile);
        fread(&indexPtr, sizeof(indexPtr), 1, sfile);
        fread(m_gameID, sizeof(m_gameID), 1, sfile);

        //  read index
        m_size = count;
        m_max = m_size;
        uint32_t *index = new uint32_t[m_size];
        fseek(sfile, indexPtr, SEEK_SET);
        fread(index, sizeof(uint32_t) * m_size, 1, sfile);
        m_scripts = std::make_unique<CScript *[]>(m_size);

        // read individual scripts
        for (int i = 0; i < count; ++i)
        {
            m_scripts[i] = new CScript;
            fseek(sfile, index[i], SEEK_SET);
            m_scripts[i]->read(sfile);
        }
        delete[] index;
        fclose(sfile);
    }
    else
    {
        m_lastError = "can't open file";
    }
    return sfile != nullptr;
}

/// @brief
/// @param filename
/// @return
bool CScriptArch::write(const char *filename)
{
    FILE *tfile = fopen(filename, "wb");
    if (tfile)
    {
        uint16_t version = 0;
        fwrite(SIGNATURE, SIGNATURE_SIZE, 1, tfile);
        fwrite(&version, sizeof(version), 1, tfile);
        fwrite(&m_size, sizeof(uint16_t), 1, tfile);
        uint32_t tmp = 0;
        fwrite(&tmp, sizeof(tmp), 1, tfile);
        fwrite(m_gameID, sizeof(m_gameID), 1, tfile);

        uint32_t *index = new uint32_t[m_size];
        printf("count:%d\n", m_size);
        for (uint32_t i = 0; i < m_size; ++i)
        {
            CScript *script = m_scripts[i];
            index[i] = ftell(tfile);
            printf("index: %.8x [entries: %d 0x%.4x]\n", index[i], script->getSize(), script->getSize());
            script->write(tfile);
        }

        uint32_t indexPtr = ftell(tfile);
        printf("indexPtr: %.8x\n", indexPtr);
        fwrite(index, sizeof(uint32_t) * m_size, 1, tfile);
        fseek(tfile, INDEXPTR_OFFSET, SEEK_SET);
        fwrite(&indexPtr, sizeof(indexPtr), 1, tfile);
        delete[] index;
        fclose(tfile);
        return true;
    }
    m_lastError = "can't open file";
    return tfile != nullptr;
}

void CScriptArch::add(CScript *script)
{
    if (m_size == m_max)
    {
        m_max += GROW_BY;
        std::unique_ptr<CScript *[]> tmp = std::make_unique<CScript *[]>(m_max);
        for (uint32_t i = 0; i < m_size; ++i)
        {
            tmp[i] = m_scripts[i];
        }
        m_scripts.swap(tmp);
    }
    m_scripts[m_size] = script;
    ++m_size;
}

void CScriptArch::add(std::unique_ptr<CActor[]> &scriptArray, uint32_t size)
{
    add(new CScript(scriptArray, size));
}

void CScriptArch::forget()
{
    if (m_scripts)
    {
        for (uint32_t i = 0; i < m_size; ++i)
        {
            delete m_scripts[i];
        }
    }
    m_scripts = nullptr;
    m_size = 0;
    m_max = 0;
}

int CScriptArch::size() const
{
    return m_size;
}

CScript *CScriptArch::operator[](int i)
{
    return m_scripts[i];
}

CScript *CScriptArch::at(int i)
{
    return (*this)[i];
}

bool CScriptArch::indexFromFile(const char *filename, uint32_t *&index, uint32_t &size)
{
    /*
   4 signature      "SCRX"
   2 version
   2 script count
   4 index offset
   */
    FILE *sfile = fopen(filename, "rb");
    if (sfile)
    {
        char signature[SIGNATURE_SIZE];
        uint16_t version = 0;
        uint16_t dwCount = 0;
        uint32_t indexOffset = 0;
        fread(signature, sizeof(signature), 1, sfile);
        fread(&version, sizeof(version), 1, sfile);
        if (memcmp(signature, SIGNATURE, sizeof(signature)) != 0)
        {
            printf("invalid signature: %s\n", signature);
            fclose(sfile);
            return false;
        }
        if (version > VERSION)
        {
            printf("invalid version: %d\n", version);
            fclose(sfile);
            return false;
        }
        fread(&dwCount, sizeof(dwCount), 1, sfile);
        fread(&indexOffset, sizeof(indexOffset), 1, sfile);
        fseek(sfile, indexOffset, SEEK_SET);
        index = new uint32_t[dwCount];
        fread(index, dwCount * sizeof(uint32_t), 1, sfile);
        size = dwCount;
        fclose(sfile);
    }
    return sfile != nullptr;
}

bool CScriptArch::indexFromMemory(const uint8_t *data, uint32_t *&index, uint32_t &size)
{
    char signature[SIGNATURE_SIZE]{};
    uint16_t version = 0;
    uint16_t dwCount = 0;
    uint32_t indexOffset = 0;
    uint8_t *p = const_cast<uint8_t *>(data);
    auto memread = [&p](auto __m__, auto __s__)
    {
        memcpy(p, __m__, __s__);
        p += __s__;
    };
    memread(signature, sizeof(signature));
    memread(&version, sizeof(version));
    if (memcmp(signature, SIGNATURE, sizeof(signature)) != 0)
    {
        printf("invalid signature: %s\n", signature);
        return false;
    }
    if (version > VERSION)
    {
        printf("invalid version: %d\n", version);
        return false;
    }
    memread(&dwCount, sizeof(dwCount));
    memread(&indexOffset, sizeof(indexOffset));
    p = const_cast<uint8_t *>(data + indexOffset);
    index = new uint32_t[dwCount];
    memread(index, dwCount * sizeof(uint32_t));
    size = dwCount;
    return true;
}

CScript *CScriptArch::removeAt(int i)
{
    CScript *t = m_scripts[i];
    for (uint32_t j = i; j < m_size - 1; ++j)
    {
        m_scripts[j] = m_scripts[j + 1];
    }
    --m_size;
    return t;
}

const char *CScriptArch::lastError()
{
    return m_lastError.c_str();
}

void CScriptArch::insertAt(int i, CScript *script)
{
    if (m_size == m_max)
    {
        m_max += GROW_BY;
        std::unique_ptr<CScript *[]> tmp = std::make_unique<CScript *[]>(m_max);
        for (uint32_t i = 0; i < m_size; ++i)
        {
            tmp[i] = m_scripts[i];
        }
        m_scripts.swap(tmp);
    }

    for (int j = m_size; j > i; --j)
    {
        m_scripts[j] = m_scripts[j - 1];
    }
    m_scripts[i] = script;
    ++m_size;
}

const char *CScriptArch::gameID()
{
    return m_gameID;
}

void CScriptArch::setGameID(const char *id)
{
    memcpy(m_gameID, id, sizeof(m_gameID));
}
