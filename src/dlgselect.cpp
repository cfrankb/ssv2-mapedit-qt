#include "dlgselect.h"
#include "ui_dlgselect.h"
#include "shared/qtgui/qfilewrap.h"
#include "shared/FrameSet.h"
#include "shared/Frame.h"
#include "shared/qtgui/qthelper.h"
#include "script.h"
#include "mapfile.h"

CDlgSelect::CDlgSelect(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CDlgSelect)
{
    ui->setupUi(this);
    m_player = new CFrameSet;
    m_frameSet = new CFrameSet;
    preloadTiles(m_player, ":/data/annie.obl");
    m_mapFile = nullptr;
}

CDlgSelect::~CDlgSelect()
{
    delete ui;
    delete m_frameSet;
    delete m_player;
}

void CDlgSelect::updatePreview(CScript *map)
{
    if (map == nullptr) {
        ui->sPreview->setText("Map is NULL.");
        return;
    }

    CActor *player = nullptr;
    int i = map->findPlayerIndex();
    if (i != CScript::NOT_FOUND)
    {
        CActor &entry = (*map)[i];
        player = &entry;
        entry.aim = CActor::AIM_DOWN;
    }

    bool status = true;
    std::string frameSet = std::string(":/data/") + map->tileset() + std::string(".obl");
    if (frameSet != m_preloadedTileset) {
        status = preloadTiles(m_frameSet, frameSet.c_str());
        m_preloadedTileset = frameSet;
    }

    if (!status || frameSet.size() == 0) {
        ui->sPreview->setText("Can't read tileset.");
        return;
    }

    CFrame bitmap(320,240);
    bitmap.fill(BLACK);

    const int scrLen = bitmap.len();
    const int scrHei = bitmap.hei();
    const int rows = bitmap.hei() / FNT_BLOCK_SIZE;
    const int cols = bitmap.len() / FNT_BLOCK_SIZE;
    const int hx = cols / 2;
    const int hy = rows / 2;
    const int mx = player->x < hx ? 0 : player->x - hx;
    const int my = player->y < hy ? 0 : player->y - hy;
    for (int i = 0; i < map->getSize(); ++i)
    {
        CFrame *frame{nullptr};
        const auto &entry{(*map)[i]};
        if (entry.type == TYPE_PLAYER)
        {
            frame = (*m_player)[PLAYER_FRAME_CYCLE];
        }
        else if (entry.type == TYPE_POINTS)
        {
            continue;
        }
        else if (entry.imageId >= m_frameSet->getSize())
        {
            continue;
        }
        else
        {
            frame = (*m_frameSet)[entry.imageId];
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
            for (int y = 0; y < fhei * FNT_BLOCK_SIZE; ++y)
            {
                if (sy * FNT_BLOCK_SIZE + y >= scrHei)
                    break;
                uint32_t *rgba = &bitmap.at(sx * FNT_BLOCK_SIZE, sy * FNT_BLOCK_SIZE + y);
                const uint32_t *pixel = &frame->at(offsetX * FNT_BLOCK_SIZE, offsetY * FNT_BLOCK_SIZE + y);
                for (int x = 0; x < flen * FNT_BLOCK_SIZE; ++x)
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

   // bitmap.shrink();
    QPixmap pixmap = frame2pixmap(bitmap);
    ui->sPreview->setPixmap(pixmap);
}

bool CDlgSelect::preloadTiles(CFrameSet * fs, const char *filename)
{
    QFileWrap file;
    fs->forget();
    qDebug("reading tiles: %s", filename);
    if (file.open(filename, "rb")) {
        if (fs->extract(file)) {
            qDebug("extracted: %d", fs->getSize());
        }
        file.close();
        return true;
    }
    return false;
}

void CDlgSelect::init(const QString s, CMapFile *mf)
{
    m_mapFile = mf;
    ui->sSelect_Maps->setText(s);
    QStringList list;
    for (int i=0; i < mf->size(); ++i) {
        list.append(tr("map %1 : %2").arg(i+1,2,10,QChar('0')).arg(mf->at(i)->name().c_str()));
    }
    ui->cbSelect_Maps->addItems(list);
    ui->cbSelect_Maps->setCurrentIndex(mf->currentIndex());
}

void CDlgSelect::on_cbSelect_Maps_currentIndexChanged(int index)
{
    updatePreview(m_mapFile->at(index));
}

int CDlgSelect::index()
{
    return ui->cbSelect_Maps->currentIndex();
}
