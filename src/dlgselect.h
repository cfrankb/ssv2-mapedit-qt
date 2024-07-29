#ifndef DLGSELECT_H
#define DLGSELECT_H

#include <QDialog>

class CFrameSet;
class CScript;
class CMapFile;

namespace Ui {
class CDlgSelect;
}

class CDlgSelect : public QDialog
{
    Q_OBJECT

public:
    explicit CDlgSelect(QWidget *parent = nullptr);
    ~CDlgSelect();
    void init(const QString s, CMapFile *mf);
    int index();

private slots:
    void on_cbSelect_Maps_currentIndexChanged(int index);

private:
    enum {
        BLACK = 0xff000000,
        ALPHA = 0xff000000,
        FNT_BLOCK_SIZE = 8,
        PLAYER_FRAME_CYCLE = 8
    };
    void updatePreview(CScript *map);
    bool preloadTiles(CFrameSet *fs, const char *filename);
    Ui::CDlgSelect *ui;
    CFrameSet *m_player = nullptr;
    CFrameSet *m_frameSet = nullptr;
    CMapFile *m_mapFile = nullptr;
    std::string m_preloadedTileset = "";
};

#endif // DLGSELECT_H
