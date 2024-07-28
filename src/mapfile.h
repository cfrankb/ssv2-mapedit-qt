#ifndef CMAPFILE_H
#define CMAPFILE_H

#include <QString>
#include "scriptarch.h"

class CMap;
class CScript;

class CMapFile : public CScriptArch
{
public:
    CMapFile();
    virtual ~CMapFile();
    bool read();
    bool write();
    QString filename();
    void setFilename(const QString);
    CScript *map();
    void setDirty(bool b);
    bool isDirty();
    bool isUntitled();
    void setCurrentIndex(int i);
    int currentIndex();
    bool isMulti();
    void forget();
    bool isWrongExt();
    CScript *removeAt(int i);

protected:
    int m_currIndex = 0;
    bool m_dirty = false;
    QString m_filename;
};

#endif // CMAPFILE_H
