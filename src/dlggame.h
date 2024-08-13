#ifndef DLGGAME_H
#define DLGGAME_H

#include <QDialog>

namespace Ui {
class CDlgGame;
}

class CDlgGame : public QDialog
{
    Q_OBJECT

public:
    explicit CDlgGame(QWidget *parent = nullptr);
    QString gameID();
    void setGameID(const QString &gameID);
    ~CDlgGame();

private slots:
    void on_eGameID_textChanged(const QString &);

private:
    Ui::CDlgGame *ui;
    enum {
      GAMEID_SIZE = 4,
    };
    void updateButtons();
};

#endif // DLGGAME_H
