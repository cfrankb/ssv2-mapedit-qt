#include "dlgeditentry.h"
#include "ui_dlgeditentry.h"
#include "actor.h"
#include "shared/FrameSet.h"
#include "shared/Frame.h"
#include "shared/qtgui/qfilewrap.h"
#include "shared/qtgui/qthelper.h"

CDlgEditEntry::CDlgEditEntry(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CDlgEditEntry)
{
    ui->setupUi(this);
    auto size = this->size();
    this->setMaximumHeight(size.height());
    this->setMaximumWidth(size.width());
    this->setMinimumHeight(size.height());
    this->setMinimumWidth(size.width());
    m_tiles = new CFrameSet;
    ui->eU1->setMaxLength(2);
    ui->eU2->setMaxLength(2);
}

void CDlgEditEntry::fillCombos()
{
    for (const auto & kv: m_config.types){
        ui->cbType->addItem(QString(kv.second.c_str()), kv.first);
    }
    for (const auto & kv: m_config.tasks){
        ui->cbTask->addItem(QString(kv.second.c_str()), kv.first);
    }
}

void CDlgEditEntry::init(CActor & entry, const std::string &tileset)
{
    if (!readConfig(m_config, ":/data/vlamits2.cfg")) {
        qDebug("can't read %s", ":/data/vlamits2.cfg");
    }

    QString filename{QString(":/data/%1.stx").arg(tileset.c_str())};
    if (!readConfig(m_config, filename.toStdString().c_str())) {
        qDebug("can't read %s", filename.toStdString().c_str());
    }
    fillCombos();

    m_entry = entry;
    char t[16];
    sprintf(t, "%.2x", m_entry.u1);
    ui->eU1->setText(t);
    sprintf(t, "%.2x", m_entry.u2);
    ui->eU2->setText(t);

    int i = ui->cbType->findData(QVariant(m_entry.type));
    if (i == INVALID)
    {
        sprintf(t, "??? 0x%.x", m_entry.type);
        ui->cbType->addItem(t, m_entry.type);
        ui->cbType->setCurrentIndex(ui->cbType->count() - 1);
    } else {
        ui->cbType->setCurrentIndex(i);
    }

    i = ui->cbTask->findData(QVariant(m_entry.task));
    if (i == INVALID)
    {
        sprintf(t, "??? 0x%.x", m_entry.task);
        ui->cbTask->addItem(t, m_entry.task);
        ui->cbTask->setCurrentIndex(ui->cbTask->count() - 1);
    }
    else {
        ui->cbTask->setCurrentIndex(i);
    }

    loadTileset(tileset);
    updateImage();
}

CDlgEditEntry::~CDlgEditEntry()
{
    delete ui;
    delete m_tiles;
}

void CDlgEditEntry::updateImage()
{
    char t[16];
    sprintf(t, "0x%.4x", m_entry.imageId);
    ui->sImageId->setText(t);

    CFrame *frame = m_entry.imageId < m_tiles->getSize() ? (*m_tiles)[m_entry.imageId] : nullptr;
    if (frame) {
        ui->sImage->setPixmap(frame2pixmap(*frame));
    } else {
        ui->sImage->setText(tr("Not available"));
    }
    m_config.images.count(m_entry.imageId) > 0 ?
        ui->sImageName->setText(m_config.images[m_entry.imageId].c_str()) :
        ui->sImageName->setText(tr("???"));
}

void CDlgEditEntry::on_btnPrev_clicked()
{
    decltype(m_entry.imageId) t = m_entry.imageId;
    m_entry.imageId = t ? --t : m_tiles->getSize() - 1;
    updateImage();
}

void CDlgEditEntry::on_btnNext_clicked()
{
    decltype(m_entry.imageId) t = m_entry.imageId;
    m_entry.imageId = t >= m_tiles->getSize() - 1  ? 0 : ++t;
    updateImage();
}

void CDlgEditEntry::loadTileset(const std::string &tileset)
{
    QString filename{QString(":/data/%1.obl").arg(tileset.c_str())};
    QFileWrap file;
    if (file.open(filename, "rb")) {
        m_tiles->read(file);
        file.close();
        qDebug("tiles: %d", m_tiles->getSize());
    }
}

const CActor & CDlgEditEntry:: value()
{
    m_entry.type = ui->cbType->currentData().toUInt();
    m_entry.task = ui->cbTask->currentData().toUInt();
    m_entry.u1 = std::strtoul(ui->eU1->text().toStdString().c_str(), nullptr, 16);
    m_entry.u2 = std::strtoul(ui->eU2->text().toStdString().c_str(), nullptr, 16);
    return m_entry;
}
