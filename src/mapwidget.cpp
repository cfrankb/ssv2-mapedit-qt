#include <QPainter>
#include <QScrollBar>
#include <algorithm>
#include "mapwidget.h"
#include "mapscroll.h"
#include "Frame.h"
#include "FrameSet.h"
#include "script.h"
#include "defs.h"
#include "shared/qtgui/qfilewrap.h"

CMapWidget::CMapWidget(QWidget *parent)
    : QWidget{parent}
{
    m_tiles = new CFrameSet;
    m_player = new CFrameSet;
   // m_animator = new CAnimator();
    m_timer.setInterval(1000 / TICK_RATE);
    m_timer.start();
    preloadAssets();
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(update()));
}

CMapWidget::~CMapWidget()
{
    m_timer.stop();
}

void CMapWidget::setMap(CScript *pMap)
{
    qDebug("setMap");
    m_map = pMap;
    if (m_map->tileset() != m_tileset) {
        m_tileset = m_map->tileset();
        loadTileset();
    }
}


void CMapWidget::loadTileset()
{
    QString filename{QString(":/data/%1.obl").arg(m_tileset.c_str())};
    QFileWrap file;
    if (file.open(filename, "rb")) {
        m_tiles->read(file);
        file.close();
        qDebug("tiles: %d\n", m_tiles->getSize());
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
    const QSize widgetSize = size();
    const int width = widgetSize.width() / 2 + FNT_BLOCK_SIZE;
    const int height = widgetSize.height() / 2 + FNT_BLOCK_SIZE;
    CMapScroll *scr = static_cast<CMapScroll*>(parent());
    const int mx = scr->horizontalScrollBar()->value();
    const int my = scr->verticalScrollBar()->value();
    // animate tiles
    ++m_ticks;
    //if (m_animate && m_ticks % 3 == 0) {
     //   m_animator->animate();
   // }

    // draw screen
    CFrame bitmap(width, height);

    if (m_map) {
        drawScreen(bitmap);
        if (m_showGrid) {
           // drawGrid(bitmap);
        }
    }

    if (width !=0 && height != 0) {
        char t[80];
        sprintf( t,"mx:%d my:%d", mx, my);
        drawFont(bitmap, 0,0, t, WHITE, true);
    }

    // show the screen
    const QImage & img = QImage(reinterpret_cast<uint8_t*>(bitmap.getRGB()), bitmap.len(), bitmap.hei(), QImage::Format_RGBX8888);
    const QPixmap & pixmap = QPixmap::fromImage(img.scaled(QSize(width * 2, height * 2)));
    QPainter p(this);
    p.drawPixmap(0, 0, pixmap);
    p.end();
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
        CFrame *frame{nullptr};
        const auto &entry{(*m_map)[i]};
        if (entry.type == TYPE_PLAYER)
        {
            frame = (*m_player)[PLAYER_FRAME_CYCLE];
        }
        else if (entry.type == TYPE_POINTS)
        {
            continue;
        }
        else if (entry.imageId >= m_tiles->getSize())
        {
            continue;
        }
        else
        {
            frame = (*m_tiles)[entry.imageId];
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
       // *(asset.frameset) = new CFrameSet();
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
    for (int i = 0; i < m_map->getSize(); ++i)
    {
        CActor &entry{(*m_map)[i]};
        CFrame *frame = entry.type == TYPE_PLAYER ? (*m_player)[0] : (*m_tiles)[entry.imageId];
        if (entry.x <= x && entry.y <= y
            && entry.x + frame->len() >= x && entry.y + frame->hei() > y) {
            return i;
        }
    }
    return INVALID;
}
