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
#ifndef __SCRIPTARCH
#define __SCRIPTARCH

#include <string>
#include <cstdint>
#include <string>
#include <memory>

class CScript;
class CActor;

class CScriptArch
{
public:
    CScriptArch();
    ~CScriptArch();

    bool read(const char *filename);
    bool write(const char *filename);
    void add(std::unique_ptr<CActor[]> &scriptArray, uint32_t size);
    void add(CScript *script);
    void forget();
    int size() const;
    inline CScript *operator[](int i);
    CScript *at(int i);
    static bool indexFromFile(const char *filename, uint32_t *&index, uint32_t &size);
    static bool indexFromMemory(const uint8_t *data, uint32_t *&index, uint32_t &size);
    CScript *removeAt(int i);
    void insertAt(int i, CScript *script);
    const char *lastError();

private:
    enum
    {
        GROW_BY = 16,
        VERSION = 0,
        INDEXPTR_OFFSET = 8,
        COUNT_OFFSET = 6,
        SIGNATURE_SIZE = 4,
    };
    constexpr static const char SIGNATURE[]{"SCRX"};
    std::string m_lastError;

protected:
    uint32_t m_size;
    uint32_t m_max;
    std::unique_ptr<CScript *[]> m_scripts;
};

#endif
