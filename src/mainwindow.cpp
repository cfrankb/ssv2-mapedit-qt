#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "mapscroll.h"
#include <QCloseEvent>
#include <QMessageBox>
#include <QFileDialog>
#include <QSettings>
#include <QStatusBar>
#include <QInputDialog>
#include "script.h"
#include "mapwidget.h"
#include "dlgselect.h"
#include "debug.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    initFileMenu();
    setWindowTitle(tr("ssv2 MapEditor"));
    setStatus("blah");
    m_scrollArea = new CMapScroll;
    setCentralWidget(m_scrollArea);

    connect(m_scrollArea, SIGNAL(statusChanged(QString)), this, SLOT(setStatus(QString)));
    connect(m_scrollArea, SIGNAL(leftClickedAt(int, int)), this, SLOT(onLeftClick(int, int)));
    connect(this, SIGNAL(mapChanged(CScript *)), m_scrollArea, SLOT(newMap(CScript *)));
    connect(m_scrollArea, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(showContextMenu(const QPoint &)));

    CMapWidget *glw = dynamic_cast<CMapWidget *>(m_scrollArea->viewport());
    connect(glw, SIGNAL(mapSpoiled()), this, SLOT(dirtyMap()));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave())
    {
        event->accept();
    }
    else
    {
        event->ignore();
        return;
    }
}

bool MainWindow::isDirty()
{
    return m_doc.isDirty();
}

bool MainWindow::maybeSave()
{
    if (isDirty())
    {
        QMessageBox::StandardButton ret = QMessageBox::warning(
            this,
            m_appName,
            tr("The document has been modified.\n"
               "Do you want to save your changes?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
        if (ret == QMessageBox::Save)
            return save();
        else if (ret == QMessageBox::Cancel)
            return false;
    }
    return true;
}

void MainWindow::open(QString fileName)
{
    if (maybeSave())
    {
        if (fileName.isEmpty())
        {
            QStringList filters;
            filters.append(m_allFilter);
            QFileDialog *dlg = new QFileDialog(this, tr("Open"), "", m_allFilter);
            dlg->setAcceptMode(QFileDialog::AcceptOpen);
            dlg->setFileMode(QFileDialog::ExistingFile);
            dlg->selectFile(m_doc.filename());
            dlg->setNameFilters(filters);
            if (dlg->exec())
            {
                QStringList fileNames = dlg->selectedFiles();
                if (fileNames.count() > 0)
                {
                    fileName = fileNames[0];
                }
            }
            delete dlg;
        }
        loadFile(fileName);
    }
    updateMenus();
}

void MainWindow::loadFile(const QString &fileName)
{
    if (!fileName.isEmpty())
    {
        QString oldFileName = m_doc.filename();
        m_doc.setFilename(fileName);
        if (m_doc.read())
        {
            qDebug("size: %d", m_doc.size());
        }
        else
        {
            warningMessage(tr("error:\n") + m_doc.lastError());
            m_doc.setFilename(oldFileName);
            // update fileList
            QSettings settings;
            QStringList files = settings.value("recentFileList").toStringList();
            files.removeAll(fileName);
            settings.setValue("recentFileList", files);
        }
        updateTitle();
        updateRecentFileActions();
        reloadRecentFileActions();
        emit mapChanged(m_doc.map());
    }
}

bool MainWindow::save()
{
    QString oldFileName = m_doc.filename();
    if (m_doc.isUntitled() || m_doc.isWrongExt())
    {
        if (!saveAs())
            return false;
    }

    if (!m_doc.write() || !updateTitle())
    {
        warningMessage(tr("Can't write file"));
        m_doc.setFilename(oldFileName);
        return false;
    }

    updateRecentFileActions();
    reloadRecentFileActions();
    return true;
}

bool MainWindow::saveAs()
{
    bool result = false;
    QStringList filters;
    QString suffix = "scrx";
    QString fileName = "";

    QFileDialog *dlg = new QFileDialog(this, tr("Save as"), "", m_allFilter);
    dlg->setAcceptMode(QFileDialog::AcceptSave);

    dlg->setNameFilters(filters);
    dlg->setAcceptMode(QFileDialog::AcceptSave);
    dlg->setDefaultSuffix(suffix);
    dlg->selectFile(m_doc.filename());
    if (dlg->exec())
    {
        QStringList fileNames = dlg->selectedFiles();
        if (fileNames.count() > 0)
        {
            fileName = fileNames[0];
        }
    }

    if (!fileName.isEmpty())
    {
        m_doc.setFilename(fileName);
        result = m_doc.write();
    }

    updateTitle();
    delete dlg;
    return result;
}

void MainWindow::warningMessage(const QString & message)
{
    QMessageBox::warning(this, m_appName, message);
}

void MainWindow::setDocument(const QString & fileName)
{
    m_doc.setFilename(fileName);
    m_doc.read();
}

bool MainWindow::updateTitle()
{
    QString file;
    if (m_doc.filename().isEmpty())
    {
        file = tr("untitled");
    }
    else
    {
        file = QFileInfo(m_doc.filename()).fileName();
    }
    m_doc.setDirty(false);
    setWindowTitle(tr("%1[*] - %2").arg(file, m_appName));
    return true;
}

void MainWindow::updateMenus()
{
    int index = m_doc.currentIndex();
    ui->actionMap_Previous->setEnabled(index > 0);
    ui->actionMap_Next->setEnabled(index < m_doc.size() - 1);
    ui->actionMap_Delete->setEnabled(m_doc.size() > 1);
    ui->actionMap_Move->setEnabled(m_doc.size() > 1);
    ui->actionMap_Go_to->setEnabled(m_doc.size() > 1);
    ui->actionMap_First->setEnabled(index > 0);
    ui->actionMap_Last->setEnabled(index < m_doc.size() - 1);
    ui->actionMap_Rename->setEnabled(m_doc.size() != 0);
}

void MainWindow::setStatus(const QString msg)
{
    ui->statusbar->showMessage(msg);
}

void MainWindow::initFileMenu()
{
    // gray out the open recent `nothin' yet`
    ui->actionNothing_yet->setEnabled(false);
    for (int i = 0; i < MAX_RECENT_FILES; i++)
    {
        m_recentFileActs[i] = new QAction(this);
        m_recentFileActs[i]->setVisible(false);
        ui->menuRecent_Maps->addAction(m_recentFileActs[i]);
        connect(m_recentFileActs[i], SIGNAL(triggered()),
                this, SLOT(openRecentFile()));
    }
    reloadRecentFileActions();
    // connect the File->Quit to the close app event
    connect(ui->actionFile_Exit, SIGNAL(triggered()), this, SLOT(close()));
    ui->actionFile_Exit->setMenuRole(QAction::QuitRole);
}

void MainWindow::reloadRecentFileActions()
{
    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();
    const int numRecentFiles = qMin(files.size(), static_cast<int>(MAX_RECENT_FILES));
    for (int i = 0; i < numRecentFiles; ++i)
    {
        QString text = tr("&%1 %2").arg(i + 1).arg(QFileInfo(files[i]).fileName());
        m_recentFileActs[i]->setText(text);
        m_recentFileActs[i]->setData(files[i]);
        m_recentFileActs[i]->setVisible(true);
        m_recentFileActs[i]->setStatusTip(files[i]);
    }
    ui->actionNothing_yet->setVisible(numRecentFiles == 0);
}

void MainWindow::updateRecentFileActions()
{
    QSettings settings;
    QStringList files = settings.value("recentFileList").toStringList();
    QString fileName = m_doc.filename();
    files.removeAll(fileName);
    if (!fileName.isEmpty())
    {
        files.prepend(fileName);
        while (files.size() > MAX_RECENT_FILES)
            files.removeLast();
    }
    settings.setValue("recentFileList", files);
}

void MainWindow::openRecentFile()
{
    QAction *action = qobject_cast<QAction *>(sender());
    if (action)
    {
        open(action->data().toString());
    }
    updateMenus();
}

void MainWindow::initToolBar()
{
    /*
    ui->toolBar->setIconSize(QSize(16, 16));
    ui->toolBar->addAction(ui->actionFile_New_File);
    ui->toolBar->addAction(ui->actionFile_Open);
    ui->toolBar->addAction(ui->actionFile_Save);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(ui->actionEdit_ResizeMap);
    ui->toolBar->addAction(ui->actionEdit_Previous_Map);
    ui->toolBar->addAction(ui->actionEdit_Next_Map);
    ui->toolBar->addAction(ui->actionEdit_First_Map);
    ui->toolBar->addAction(ui->actionEdit_Last_Map);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(ui->actionEdit_Add_Map);
    // ui->toolBar->addAction(ui->actionClear_Map);
    ui->toolBar->addAction(ui->actionEdit_Delete_Map);
    ui->toolBar->addSeparator();
    ui->toolBar->addAction(ui->actionTools_Paint);
    ui->toolBar->addAction(ui->actionTools_Erase);
    ui->toolBar->addAction(ui->actionTools_Select);

    m_toolGroup = new QActionGroup(this);
    m_toolGroup->addAction(ui->actionTools_Paint);
    m_toolGroup->addAction(ui->actionTools_Erase);
    m_toolGroup->addAction(ui->actionTools_Select);
    m_toolGroup->setExclusive(true);
    ui->actionTools_Paint->setChecked(true);
    ui->actionTools_Paint->setData(TOOL_PAINT);
    ui->actionTools_Erase->setData(TOOL_ERASE);
    ui->actionTools_Select->setData(TOOL_SELECT);

    QAction *actionToolBar = ui->toolBar->toggleViewAction();
    actionToolBar->setText(tr("ToolBar"));
    actionToolBar->setStatusTip(tr("Show or hide toolbar"));
    ui->menuView->addAction(actionToolBar);
*/
}

void MainWindow::on_actionFile_Open_triggered()
{
    open("");
}


void MainWindow::on_actionFile_Save_triggered()
{
    save();
}

void MainWindow::on_actionFile_New_File_triggered()
{
    if (maybeSave())
    {
        m_doc.setFilename("");
        m_doc.forget();
        CScript *script = new CScript();
        m_doc.add(script);
        updateTitle();
        emit mapChanged(m_doc.map());
        updateMenus();
    }
}


void MainWindow::on_actionFile_Save_as_triggered()
{
    saveAs();
    updateRecentFileActions();
    reloadRecentFileActions();
}

void MainWindow::onLeftClick(int x, int y) {
    if ((x >= 0) && (y >= 0) && (x < MAX_AXIS) && (y < MAX_AXIS))
    {
        CScript *map = m_doc.map();
        if (!map) {
            return;
        }

        //CActor * actor = widget->at(x, y);


        /*
        const uint8_t tile = m_doc.map()->at(x, y);
        uint8_t newTileId = tile;
        switch (currentTool())
        {
        case TOOL_PAINT:
            newTileId = m_currTile;
            break;
        case TOOL_ERASE:
            newTileId = 0;
            break;
        case TOOL_SELECT:
            m_currTile = m_doc.map()->at(x, y);
            emit newTile(m_currTile);
        }

        if (newTileId != tile)
        {
            m_doc.map()->at(x, y) = newTileId;
            m_doc.setDirty(true);
        }*/
    }
}

void MainWindow::showContextMenu(const QPoint &)
{

}

void MainWindow::on_actionExport_Map_Object_List_triggered()
{
    CScript *map = m_doc.map();
    if (!map) {
        return;
    }
    debugLevel("debug.log", map);
}

void MainWindow::dirtyMap()
{
    m_doc.setDirty(true);
}


void MainWindow::on_actionEdit_Cut_triggered()
{
    CScript *map = m_doc.map();
    if (!map) {
        return;
    }
    auto widget = reinterpret_cast<CMapWidget*>(m_scrollArea->viewport());
    int selected = widget->selected();
    if (selected != INVALID) {
        m_clipboard = {map->tileset(), selected, CActor{(*map)[selected]}};
        map->removeAt(selected);
        widget->select(INVALID);
        m_doc.setDirty(true);
    }
}


void MainWindow::on_actionEdit_Copy_triggered()
{
    CScript *map = m_doc.map();
    if (!map) {
        return;
    }
    auto widget = reinterpret_cast<CMapWidget*>(m_scrollArea->viewport());
    int selected = widget->selected();
    if (selected != INVALID) {
        m_clipboard = {map->tileset(), selected, CActor{(*map)[selected]}};
    }
}


void MainWindow::on_actionEdit_Paste_triggered()
{
    CScript *map = m_doc.map();
    if (!map) {
        return;
    }
    auto widget = reinterpret_cast<CMapWidget*>(m_scrollArea->viewport());
    if (m_clipboard.entryID != INVALID
        && m_clipboard.tileset == map->tileset()) {
        CActor actor = m_clipboard.selected;
        actor.x += 1;
        actor.y += 1;
        widget->select(map->add(actor));
        m_doc.setDirty(true);
    }
}


void MainWindow::on_actionEdit_Delete_triggered()
{
    CScript *map = m_doc.map();
    if (!map) {
        return;
    }
    auto widget = reinterpret_cast<CMapWidget*>(m_scrollArea->viewport());
    int selected = widget->selected();
    if (selected != INVALID) {
        map->removeAt(selected);
        widget->select(INVALID);
        m_doc.setDirty(true);
    }
    updateMenus();
}


void MainWindow::on_actionMap_Previous_triggered()
{
    int i = m_doc.currentIndex();
    if (i > 0) {
        m_doc.setCurrentIndex(--i);
        emit mapChanged(m_doc.map());
    }
    updateMenus();
}

void MainWindow::on_actionMap_Next_triggered()
{
    int i = m_doc.currentIndex();
    if (i < m_doc.size() -1) {
        m_doc.setCurrentIndex(++i);
        emit mapChanged(m_doc.map());
    }
    updateMenus();
}


void MainWindow::on_actionMap_First_triggered()
{
    int i = m_doc.currentIndex();
    if (i > 0) {
        m_doc.setCurrentIndex(0);
        emit mapChanged(m_doc.map());
    }
    updateMenus();
}


void MainWindow::on_actionMap_Last_triggered()
{
    int i = m_doc.currentIndex();
    if (i < m_doc.size() -1) {
        m_doc.setCurrentIndex(m_doc.size() -1);
        emit mapChanged(m_doc.map());
    }
    updateMenus();
}


void MainWindow::on_actionMap_Add_new_triggered()
{
    m_doc.add(new CScript());
    m_doc.setCurrentIndex(m_doc.size() -1);
    m_doc.setDirty(true);
    emit mapChanged(m_doc.map());
    updateMenus();
}


void MainWindow::on_actionMap_Insert_triggered()
{
}


void MainWindow::on_actionMap_Delete_triggered()
{
    if (m_doc.size() != 0) {
        int i = m_doc.currentIndex();
        CScript *script = m_doc.removeAt(i);
        delete script;
        m_doc.setDirty(true);
        emit mapChanged(m_doc.map());
    }
    updateMenus();
}


void MainWindow::on_actionMap_Go_to_triggered()
{
    int currIndex = m_doc.currentIndex();
    if (m_doc.size() != 0) {
        CDlgSelect dlg;
        dlg.setWindowTitle(tr("Go to Map ..."));
        dlg.init(tr("Goto Map"), &m_doc);
        if (dlg.exec() == QDialog::Accepted && dlg.index() != currIndex)
        {
            int i = dlg.index();
            m_doc.setCurrentIndex(i);
            emit mapChanged(m_doc.map());
            updateMenus();
        }
    }
}


void MainWindow::on_actionMap_Rename_triggered()
{
    if (m_doc.size() != 0) {
        bool ok;
        QString text = QInputDialog::getText(this, tr("Rename Map"),
                                             tr("Name:"), QLineEdit::Normal,
                                             m_doc.map()->name().c_str(), &ok);
        text = text.trimmed().mid(0, 254);
        if (ok && strcmp(text.toLatin1(), m_doc.map()->name().c_str()) != 0)
        {
            m_doc.map()->setName(text.toLatin1().toStdString());
            m_doc.setDirty(true);
        }
    }
}


void MainWindow::on_actionMap_Move_triggered()
{
    int currIndex = m_doc.currentIndex();
    if (m_doc.size() != 0) {
        CDlgSelect dlg;
        dlg.setWindowTitle(tr("Go to Map ..."));
        dlg.init(tr("Select map"), &m_doc);
        if (dlg.exec() == QDialog::Accepted && dlg.index() != currIndex)
        {
            int i = dlg.index();
            m_doc.setCurrentIndex(i);
            emit mapChanged(m_doc.map());
            updateMenus();
        }
    }
}

