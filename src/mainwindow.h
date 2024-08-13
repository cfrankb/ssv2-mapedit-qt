#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <vector>
#include "mapfile.h"
#include "actor.h"

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
    void dirtyMap();
    void openRecentFile();
    void loadFile(const QString & filename);
    void setStatus(const QString str);
    void onLeftClick(int, int);
    void showContextMenu(const QPoint &pos);
    void shiftUp();
    void shiftDown();
    void shiftLeft();
    void shiftRight();
    void updateMenus();
    void on_actionFile_Open_triggered();
    void on_actionFile_Save_triggered();
    void on_actionFile_New_File_triggered();
    void on_actionFile_Save_as_triggered();
    void on_actionExport_Map_Object_List_triggered();
    void on_actionEdit_EditObject_triggered();
    void on_actionEdit_Cut_triggered();
    void on_actionEdit_Copy_triggered();
    void on_actionEdit_Paste_triggered();
    void on_actionEdit_Delete_triggered();
    void on_actionEdit_Insert_triggered();
    void on_actionEdit_Move_to_back_triggered();
    void on_actionEdit_Move_to_front_triggered();
    void on_actionEdit_Preferences_triggered();
    void on_actionMap_Previous_triggered();
    void on_actionMap_Next_triggered();
    void on_actionMap_First_triggered();
    void on_actionMap_Last_triggered();
    void on_actionMap_Add_new_triggered();
    void on_actionMap_Insert_triggered();
    void on_actionMap_Delete_triggered();
    void on_actionMap_Go_to_triggered();
    void on_actionMap_Rename_triggered();
    void on_actionMap_Move_triggered();
    void on_actionMap_Clear_triggered();
    void on_actionHelp_About_Qt_triggered();
    void on_actionHelp_About_triggered();
    void on_actionMap_Sort_Objects_triggered();

private:
    enum:int {
        MAX_RECENT_FILES = 8,
        MAX_AXIS = 255,
        INVALID = -1,
        FNT_BLOCK_SIZE = 8,
        SCREEN_PARTION = 2 * FNT_BLOCK_SIZE,
    };
    Ui::MainWindow *ui;

    using clipboard_t = struct {
        std::string tileset;
        std::vector<CActor> selected;
    };

    CMapFile m_doc;
    QString m_appName = tr("mapedit");
    QString m_allFilter = tr("SSV2 Maps (*.scrx)");
    QAction *m_recentFileActs[MAX_RECENT_FILES];
    CMapScroll *m_scrollArea;
    clipboard_t m_clipboard{"", {}};
    int m_entryID = INVALID;

    virtual void closeEvent(QCloseEvent *event) override;
    bool isDirty();
    bool maybeSave();
    void open(QString fileName);
    bool save();
    bool saveAs();
    void warningMessage(const QString & message);
    void setDocument(const QString & fileName);
    bool updateTitle();
    void initFileMenu();
    void reloadRecentFileActions();
    void initToolBar();
    void updateRecentFileActions();
    void initMapShortcuts();
    bool editGameOptions();
    void addNewMap();

};
#endif // MAINWINDOW_H
