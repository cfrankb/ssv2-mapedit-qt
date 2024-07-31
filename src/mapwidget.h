#ifndef CMAPWIDGET_H
#define CMAPWIDGET_H

class CFrame;
class CFrameSet;
class CScript;
class CActor;
class CSelection;

#include <QWidget>
#include <QTimer>
#include <string>

class CMapWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CMapWidget(QWidget *parent = nullptr);
    virtual ~CMapWidget();
    void setMap(CScript *pMap);
    int at(int x, int y);
    void select(int selected);
    int selected();
    void clearSelection();
    void translate(int tx, int ty);
    CSelection *selection();

signals:
    void mapSpoiled();

protected slots:
    void showGrid(bool show);
    void setAnimate(bool val);

protected:

    virtual void paintEvent(QPaintEvent *) ;
    enum:uint32_t {
        MAX_AXIS = 256,
        FNT_BLOCK_SIZE = 8,
        FONT_SIZE = 8,
        NO_ANIMZ = 255,
        TICK_RATE = 24,
        TILE_SIZE = 16,
        ALPHA  = 0xff000000,
        WHITE  = 0x00ffffff | ALPHA,
        YELLOW = 0x0000ffff | ALPHA,
        PURPLE = 0x00ff00ff | ALPHA,
        BLACK  = 0x00000000 | ALPHA,
        GREEN  = 0x0000ff00 | ALPHA,
        LIME   = 0x0034ebb1 | ALPHA,
        BLUE   = 0x00ff0000 | ALPHA,
        DARKBLUE = 0x00440000 | ALPHA,
        LIGHTSLATEGRAY= 0x00998877 | ALPHA,
        LIGHTGRAY= 0x00DCDCDC | ALPHA,
        GRIDCOLOR = 0x00bfa079 | ALPHA,
        PLAYER_FRAME_CYCLE = 8,
    };

    enum {
        INVALID = -1,
    };

    void preloadAssets();
    void loadTileset();
    inline void drawScreen(CFrame &bitmap);
    inline void drawFont(CFrame & frame, int x, int y, const char *text, const uint32_t color, const bool alpha);
    //inline void drawTile(CFrame & bitmap, const int x, const int y, CFrame & tile, const bool alpha);
    //inline void drawGrid(CFrame & bitmap);
    void drawSelectionRect(CFrame &bitmap, int entryID);
    CFrame * fromEntry(const CActor &entry);

    QTimer m_timer;
    CFrameSet *m_tiles = nullptr;
    CFrameSet *m_player = nullptr;
    uint8_t *m_fontData = nullptr;
    CScript *m_map = nullptr;
    bool m_showGrid = false;
    bool m_animate = false;
    uint32_t m_ticks = 0;
    std::string m_tileset;
    int m_selected = INVALID;
    CSelection *m_selection;

    friend class CMapScroll;
};

#endif // CMAPWIDGET_H
