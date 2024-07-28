#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "mapfile.h"

class QCloseEvent;
class CScript;
class CMapScroll;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


signals:
    void mapChanged(CScript *);    // notify of a map change

private slots:
    void openRecentFile();
    void loadFile(const QString & filename);
    void setStatus(const QString str);
    void onLeftClick(int, int);
    void showContextMenu(const QPoint &);
    void on_actionFile_Open_triggered();
    void on_actionFile_Save_triggered();
    void on_actionFile_New_File_triggered();
    void on_actionFile_Save_as_triggered();

private:
    enum {
        MAX_RECENT_FILES =4,
        MAX_AXIS = 256,
        INVALID = -1,
    };
    Ui::MainWindow *ui;
    CMapFile m_doc;

    QString m_appName = tr("mapedit");
    QString m_allFilter = tr("SSV2 Maps (*.scrx)");
    QAction *m_recentFileActs[MAX_RECENT_FILES];
    CMapScroll *m_scrollArea;

    virtual void closeEvent(QCloseEvent *event) override;
    bool isDirty();
    bool maybeSave();
    void open(QString fileName);
    bool save();
    bool saveAs();
    void warningMessage(const QString & message);
    void setDocument(const QString & fileName);
    bool updateTitle();
    void updateMenus();
    void initFileMenu();
    void reloadRecentFileActions();
    void initToolBar();
    void updateRecentFileActions();

};
#endif // MAINWINDOW_H
