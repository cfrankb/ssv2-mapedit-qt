#include "mapfile.h"

CMapFile::CMapFile() : CScriptArch()
{
    //m_scripts[0] = new CScript();
    //m_scripts = nullptr;
    m_size = 0;
    m_currIndex = 0;
}

CMapFile::~CMapFile()
{
}

bool CMapFile::read()
{
    m_currIndex = 0;
    return CScriptArch::read(filename().toLocal8Bit().toStdString().c_str());
}

bool CMapFile::write()
{
    return CScriptArch::write(filename().toLocal8Bit().toStdString().c_str());
}

QString CMapFile::filename()
{
    return m_filename;
}

void CMapFile::setFilename(const QString filename)
{
    m_filename = filename;
}

CScript *CMapFile::map()
{
    return m_scripts[m_currIndex];
}

void CMapFile::setDirty(bool b)
{
    m_dirty = b;
}

bool CMapFile::isDirty()
{
    return m_dirty;
}

bool CMapFile::isUntitled()
{
    return m_filename.isEmpty();
}

void CMapFile::setCurrentIndex(int i)
{
    m_currIndex = i;
}

int CMapFile::currentIndex()
{
    return m_currIndex;
}

bool CMapFile::isMulti()
{
    return m_size > 1;
}

bool CMapFile::isWrongExt()
{
    if (isMulti())
    {
        return !m_filename.endsWith(".mapz");
    }
    else
    {
        return !m_filename.endsWith(".dat");
    }
}

void CMapFile::forget()
{
    CScriptArch::forget();
    m_currIndex = 0;
}

CScript *CMapFile::removeAt(int i)
{
    CScript *map = CScriptArch::removeAt(i);
    if (currentIndex() >= m_size)
    {
        setCurrentIndex(m_size - 1);
    }
    return map;
}


