#include <QPainter>
#include <QScrollBar>
#include <algorithm>
#include <cstdlib>
#include "mapwidget.h"
#include "mapscroll.h"
#include "Frame.h"
#include "FrameSet.h"
#include "script.h"
#include "defs.h"
#include "selection.h"
#include "shared/qtgui/qfilewrap.h"

CMapWidget::CMapWidget(QWidget *parent)
    : QWidget{parent}
{
    m_tiles = new CFrameSet;
    m_player = new CFrameSet;
    m_timer.setInterval(1000 / TICK_RATE);
    m_timer.start();
    preloadAssets();
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(update()));
    m_selection = new CSelection;
    m_selectRect.show = false;
}

CMapWidget::~CMapWidget()
{
    m_timer.stop();

    if (m_tiles) {
        delete m_tiles;
    }

    if (m_player) {
        delete m_player;
    }

    if (m_selection) {
        delete m_selection;
    }
}

void CMapWidget::setMap(CScript *pMap)
{
    m_map = pMap;
    if (m_map->tileset() != m_tileset) {
        m_tileset = m_map->tileset();
        loadTileset();
    }
    m_selection->clear();
    m_selectRect.show = false;
}

bool CMapWidget::loadTileset()
{
    QString filename{QString(":/data/%1.obl").arg(m_tileset.c_str())};
    QFileWrap file;
    if (file.open(filename, "rb")) {
        m_tiles->read(file);
        file.close();
        qDebug("tiles: %d\n", m_tiles->getSize());
        return true;
    } else {
        qDebug("failed to read: %s", filename.toStdString().c_str());
        m_tiles->forget();
        return false;
    }
}

void CMapWidget::showGrid(bool show)
{
    m_showGrid = show;
}

void CMapWidget::setAnimate(bool val)
{
    m_animate = val;
}

void CMapWidget::paintEvent(QPaintEvent *)
{
    char t[80];
    const QSize widgetSize = size();
    const int width = widgetSize.width() / 2 + FNT_BLOCK_SIZE;
    const int height = widgetSize.height() / 2 + FNT_BLOCK_SIZE;
    CMapScroll *scr = static_cast<CMapScroll*>(parent());
    const int mx = scr->horizontalScrollBar()->value();
    const int my = scr->verticalScrollBar()->value();
    ++m_ticks;

    // draw screen
    CFrame bitmap(width, height);

    if (m_map && !m_tiles->getSize()) {
        sprintf(t, "failed to load: %s", m_tileset.c_str());
        drawFont(bitmap, 0,0, t, WHITE, true );
    } else if (m_map) {
        drawScreen(bitmap);
        if (m_showGrid) {
           // drawGrid(bitmap);
        }
        const uint32_t color{static_cast<uint32_t>(rand())};
        for (int i=0; i < m_selection->getSize(); ++i) {
            int selected = m_selection->getIndex(i);
            drawSelectionRect(bitmap, selected, color);
        }
        if (m_selectRect.show) {
            drawSelectionRect(bitmap, m_selectRect.x, m_selectRect.y, m_selectRect.dx, m_selectRect.dy, color);
        }

        if (width !=0 && height != 0) {
            sprintf(t,"mx:%d my:%d", mx, my);
            drawFont(bitmap, 0,0, t, WHITE, true);
        }
    } else {
        drawCheckers(bitmap);
    }

    // show the screen
    const QImage & img = QImage(reinterpret_cast<uint8_t*>(bitmap.getRGB()), bitmap.len(), bitmap.hei(), QImage::Format_RGBX8888);
    const QPixmap & pixmap = QPixmap::fromImage(img.scaled(QSize(width * 2, height * 2)));
    QPainter p(this);
    p.drawPixmap(0, 0, pixmap);
    p.end();
}

void CMapWidget::drawSelectionRect(CFrame &bitmap, const int entryID, const uint32_t color)
{
    if (entryID == INVALID || !m_map) {
        return;
    }
    const auto &entry{(*m_map)[entryID]};
    CFrame *frame = fromEntry(entry);
    const int fcols = frame->len() / FNT_BLOCK_SIZE;
    const int frows = frame->hei() / FNT_BLOCK_SIZE;
    drawSelectionRect(bitmap, entry.x, entry.y, fcols, frows, color);
}

void CMapWidget::drawSelectionRect(CFrame &bitmap, const int x, const int y, const int fcols, const int frows, const uint32_t color)
{
    CMapScroll *scr = static_cast<CMapScroll*>(parent());
    const int mx = scr->horizontalScrollBar()->value();
    const int my = scr->verticalScrollBar()->value();
    const int rx = x - mx;
    const int ry = y - my;
    const int maxRows = bitmap.hei() / FNT_BLOCK_SIZE;
    const int maxCols = bitmap.len() / FNT_BLOCK_SIZE;
    const int rows = std::min(maxRows, static_cast<int>(MAX_AXIS));
    const int cols = std::min(maxCols, static_cast<int>(MAX_AXIS));
    const uint scrLen = bitmap.len();
    const uint scrHei = bitmap.hei();

    if ((rx < cols) &&
        (rx + fcols > 0) &&
        (ry < rows) &&
        (ry + frows > 0))
    {
        const int offsetX = rx < 0 ? -rx : 0;
        const int offsetY = ry < 0 ? -ry : 0;
        const int flen = fcols - offsetX;
        const int fhei = frows - offsetY;
        const int sx = rx > 0 ? rx : 0;
        const int sy = ry > 0 ? ry : 0;
        const int maxX = flen * FNT_BLOCK_SIZE -1;
        const int maxY = fhei * FNT_BLOCK_SIZE -1;
        const uint baseX = sx * FNT_BLOCK_SIZE;
        const uint baseY = sy * FNT_BLOCK_SIZE;
        for (int y = 0; y <= maxY; ++y)
        {
            if (baseY + y >= scrHei)
                break;
            uint32_t *rgba = &bitmap.at(baseX, baseY + y);
            for (int x = 0; x <= maxX ; ++x)
            {
                if (baseX + x >= scrLen)
                    break;
                if (y == maxY || (y == 0 && ry >= 0) ||
                    (x == maxX || (x == 0 && rx >= 0))) {
                    rgba[x] = color;
                }
            }
        }
    }
}

CFrame * CMapWidget::fromEntry(const CActor &entry)
{
    CFrame *frame{nullptr};
    if (entry.type == TYPE_PLAYER)
    {
        frame = (*m_player)[PLAYER_FRAME_CYCLE];
    }
    else if (entry.type == TYPE_POINTS)
    {
        frame = nullptr;
    }
    else if (entry.imageId >= m_tiles->getSize())
    {
        frame = nullptr;
    }
    else
    {
        frame = (*m_tiles)[entry.imageId];
    }
    return frame;
}

void CMapWidget::drawScreen(CFrame &bitmap)
{
    const int maxRows = bitmap.hei() / FNT_BLOCK_SIZE;
    const int maxCols = bitmap.len() / FNT_BLOCK_SIZE;
    const int rows = std::min(maxRows, static_cast<int>(MAX_AXIS));
    const int cols = std::min(maxCols, static_cast<int>(MAX_AXIS));
    CMapScroll *scr = static_cast<CMapScroll*>(parent());
    const int mx = scr->horizontalScrollBar()->value();
    const int my = scr->verticalScrollBar()->value();
    const uint scrLen = bitmap.len();
    const uint scrHei = bitmap.hei();

    for (int i = 0; i < m_map->getSize(); ++i)
    {
        const auto &entry{(*m_map)[i]};
        CFrame *frame = fromEntry(entry);
        if (frame == nullptr) {
            continue;
        }
        const int fcols = frame->len() / FNT_BLOCK_SIZE;
        const int frows = frame->hei() / FNT_BLOCK_SIZE;
        const int rx = int(entry.x) - mx;
        const int ry = int(entry.y) - my;
        if ((rx < cols) &&
            (rx + fcols > 0) &&
            (ry < rows) &&
            (ry + frows > 0))
        {
            const int offsetX = rx < 0 ? -rx : 0;
            const int offsetY = ry < 0 ? -ry : 0;
            const int flen = fcols - offsetX;
            const int fhei = frows - offsetY;
            const int sx = rx > 0 ? rx : 0;
            const int sy = ry > 0 ? ry : 0;
            for (uint y = 0; y < fhei * FNT_BLOCK_SIZE; ++y)
            {
                if (sy * FNT_BLOCK_SIZE + y >= scrHei)
                    break;
                uint32_t *rgba = &bitmap.at(sx * FNT_BLOCK_SIZE, sy * FNT_BLOCK_SIZE + y);
                const uint32_t *pixel = &frame->at(offsetX * FNT_BLOCK_SIZE, offsetY * FNT_BLOCK_SIZE + y);
                for (uint x = 0; x < flen * FNT_BLOCK_SIZE; ++x)
                {
                    if (sx * FNT_BLOCK_SIZE + x >= scrLen)
                        break;
                    if (pixel[x])
                    {
                        rgba[x] = pixel[x];
                    }
                }
            }
        }
    }
}

void CMapWidget::drawFont(CFrame & frame, int x, int y, const char *text, const uint32_t color, const bool alpha)
{
    uint32_t *rgba = frame.getRGB();
    const int rowPixels = frame.len();
    const int fontSize = static_cast<int>(FONT_SIZE);
    const int fontOffset = fontSize;
    const int textSize = strlen(text);
    for (int i=0; i < textSize; ++i) {
        const uint8_t c = static_cast<uint8_t>(text[i]) - ' ';
        uint8_t *font = m_fontData + c * fontOffset;
        if (alpha) {
            for (int yy=0; yy < fontSize; ++yy) {
                uint8_t bitFilter = 1;
                for (int xx=0; xx < fontSize; ++xx) {
                    uint8_t lb = 0;
                    if (xx > 0) lb = font[xx] & (bitFilter >> 1);
                    if (yy > 0 && lb == 0) lb = font[xx - 1] & bitFilter;
                    if (font[yy] & bitFilter) {
                        rgba[ (yy + y) * rowPixels + xx + x] = color;
                    } else if (lb) {
                        rgba[ (yy + y) * rowPixels + xx + x] = BLACK;
                    }
                    bitFilter = bitFilter << 1;
                }
            }
        } else {
            for (int yy=0; yy < fontSize; ++yy) {
                uint8_t bitFilter = 1;
                for (int xx=0; xx < fontSize; ++xx) {
                    rgba[ (yy + y) * rowPixels + xx + x] = font[yy] & bitFilter ? color : BLACK;
                    bitFilter = bitFilter << 1;
                }
            }
        }
        x+= fontSize;
    }
}

void CMapWidget::preloadAssets()
{
    QFileWrap file;

    typedef struct {
        const char *filename;
        CFrameSet *frameset;
    } asset_t;

    asset_t assets[] = {
                        {":/data/annie.obl", m_player},
                        };

    for (uint i=0; i < sizeof(assets)/sizeof(assets[0]); ++i) {
        asset_t & asset = assets[i];
        if (file.open(asset.filename, "rb")) {
            qDebug("reading %s", asset.filename);
            if (asset.frameset->extract(file)) {
                qDebug("extracted: %d", asset.frameset->getSize());
            }
            file.close();
        }
    }

    const char fontName [] = ":/data/bitfont.bin";
    int size = 0;
    if (file.open(fontName, "rb")) {
        size = file.getSize();
        m_fontData = new uint8_t[size];
        file.read(m_fontData, size);
        file.close();
        qDebug("loading font: %d bytes", size);
    } else {
        qDebug("failed to open %s", fontName);
    }
}

int CMapWidget::at(int x, int y)
{
    if (!m_map) {
        return INVALID;
    }
    for (int i = m_map->getSize() - 1; i >= 0; --i)
    {
        CActor &entry{(*m_map)[i]};
        CFrame *frame = fromEntry(entry);
        if (frame == nullptr) {
            continue;
        }
        if ( (x >= entry.x) && (static_cast<uint32_t>(x) < entry.x + frame->len() / FNT_BLOCK_SIZE)
            && (y >= entry.y) && (static_cast<uint32_t>(y) < entry.y + frame->hei() / FNT_BLOCK_SIZE) ) {
            return i;
        }
    }
    return INVALID;
}

void CMapWidget::select(int selected)
{
    m_selection->addEntry((*m_map)[selected], selected);
}

void CMapWidget::translate(int tx, int ty)
{
    if (!m_map || m_selection->getSize() == 0) {
        return ;
    }
    emit mapSpoiled();
    m_selection->applyDelta(tx,ty, m_map);
}

CSelection * CMapWidget::selection() {
    return m_selection;
}

void CMapWidget::showRect(int x, int y, int dx, int dy)
{
    m_selectRect = rect_t{true, x,y, dx,dy};
}

void CMapWidget::hideRect()
{
    m_selectRect.show = false;
}

void CMapWidget::collectRect()
{
    if (m_selectRect.show) {
        selectWithin(
            m_selectRect.x, m_selectRect.y,
            m_selectRect.x + m_selectRect.dx - 1,
            m_selectRect.y + m_selectRect.dy - 1);
        hideRect();
    }
}

void CMapWidget::selectWithin(const int x1, const int y1, const int x2, const int y2)
{
    if (!m_map) {
        return;
    }

    for (int i=0; i < m_map->getSize(); ++i) {
        const CActor & entry = (*m_map)[i];
        const CFrame *frame = fromEntry(entry);
        if (!frame) {
            continue;
        }
        const int len = frame->len() / FNT_BLOCK_SIZE;
        const int hei = frame->hei() / FNT_BLOCK_SIZE;
        if (((entry.x >= x1 && entry.x <= x2)
                || (entry.x <= x1 && entry.x + len >= x1)
                || (entry.x <= x2 && entry.x + len >= x2))
            &&
            ((entry.y >= y1 && entry.y <= y2)
             || (entry.y <= y1 && entry.y + hei >= y1)
             || (entry.y <= y2 && entry.y + hei >= y2))
            )
        {
            m_selection->addEntry(entry, i);
        }
    }
}

void CMapWidget::drawCheckers(CFrame &bitmap)
{
    bitmap.fill(LIGHTBLUE);
    for (int y=0; y < bitmap.hei(); ++y) {
        for (int x=0; x < bitmap.len(); ++x) {
            if ( ((x & 16) !=0 && (y & 16)!=0) ||
                 ((x & 16) ==0 && (y & 16)==0) )
            {
                bitmap.at(x,y) = MIDBLUE;
            }
        }
    }
}
