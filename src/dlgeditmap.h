#ifndef DLGEDITMAP_H
#define DLGEDITMAP_H

#include <QDialog>

namespace Ui {
class CDlgEditMap;
}

class CDlgEditMap : public QDialog
{
    Q_OBJECT

public:
    explicit CDlgEditMap(QWidget *parent = nullptr);
    ~CDlgEditMap();
    const char *tileset();
    const char *name();
    void setName(const char *name);
    void setTileset(const char *tileset);

private slots:
    void on_eTileset_textChanged(const QString &arg1);
    void on_eName_textChanged(const QString &arg1);

private:
    enum {
        MAX_NAME = 254,
        MAX_TILESET_NAME = 12,
    };

    Ui::CDlgEditMap *ui;
    void validateFields();
};

#endif // DLGEDITMAP_H
