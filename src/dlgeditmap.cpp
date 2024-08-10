#include "dlgeditmap.h"
#include "ui_dlgeditmap.h"
#include <QDialogButtonBox>
#include <QPushButton>
#include <string>

CDlgEditMap::CDlgEditMap(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::CDlgEditMap)
{
    ui->setupUi(this);

    auto size = this->size();
    this->setMaximumHeight(size.height());
    this->setMaximumWidth(size.width());
    this->setMinimumHeight(size.height());
    this->setMinimumWidth(size.width());
    ui->eName->setMaxLength(MAX_NAME);
    ui->eTileset->setMaxLength(MAX_TILESET_NAME);
    ui->eName->setFocus();
    validateFields();
}

CDlgEditMap::~CDlgEditMap()
{
    delete ui;
}

const char *CDlgEditMap::tileset()
{
    static std::string s;
    s = ui->eTileset->text().trimmed().toStdString();
    return s.c_str();
}

const char *CDlgEditMap::name()
{
    static std::string s;
    s = ui->eName->text().trimmed().toStdString();
    return s.c_str();
}

void CDlgEditMap::setName(const char *name)
{
    ui->eName->setText(name);
    validateFields();
}

void CDlgEditMap::setTileset(const char *tileset)
{
    ui->eTileset->setText(tileset);
    validateFields();
}

void CDlgEditMap::on_eTileset_textChanged(const QString &)
{
    validateFields();
}

void CDlgEditMap::on_eName_textChanged(const QString &)
{
    validateFields();
}

void CDlgEditMap::validateFields() {

    QString name = ui->eName->text().trimmed();
    QString tileset = ui->eTileset->text().trimmed();

    ui->buttonBox->button(QDialogButtonBox::Ok)->setEnabled(
        name.length() > 0 && tileset.length() > 0);
}
