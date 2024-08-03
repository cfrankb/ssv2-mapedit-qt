#ifndef DLGEDITENTRY_H
#define DLGEDITENTRY_H

#include <QDialog>
#include "parser.h"
#include "actor.h"

class QComboBox;
class CActor;
class CFrameSet;

namespace Ui {
class CDlgEditEntry;
}

class CDlgEditEntry : public QDialog
{
    Q_OBJECT

public:
    explicit CDlgEditEntry(QWidget *parent = nullptr);
    ~CDlgEditEntry();
    void init(CActor & entry, const std::string & tileset);
    const CActor & value();

private slots:
    void on_btnPrev_clicked();
    void on_btnNext_clicked();

private:
    enum {
        INVALID = -1,
    };
    Ui::CDlgEditEntry *ui;
    void fillCombos();
    void loadTileset(const std::string &tileset);
    void updateImage();
    CActor m_entry;
    config_t m_config;
    CFrameSet* m_tiles = nullptr;
};

#endif // DLGEDITENTRY_H
