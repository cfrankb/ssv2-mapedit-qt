#include "dlggame.h"
#include "ui_dlggame.h"
#include <QPushButton>
#include <QLineEdit>

CDlgGame::CDlgGame(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CDlgGame)
{
    ui->setupUi(this);
    ui->eGameID->setMaxLength(GAMEID_SIZE);
    ui->eGameID->setFocus();
    auto size = this->size();
    this->setMaximumHeight(size.height());
    this->setMaximumWidth(size.width());
    this->setMinimumHeight(size.height());
    this->setMinimumWidth(size.width());
    updateButtons();
}

CDlgGame::~CDlgGame()
{
    delete ui;
}

QString CDlgGame::gameID()
{
    return ui->eGameID->text();
}

void CDlgGame::on_eGameID_textChanged(const QString &)
{
    updateButtons();
}

void CDlgGame::updateButtons()
{
    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(
        ui->eGameID->text().trimmed().length() == GAMEID_SIZE);
}

void CDlgGame::setGameID(const QString &gameID)
{
    ui->eGameID->setText(gameID);
}

