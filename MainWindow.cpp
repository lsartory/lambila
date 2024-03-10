#include "MainWindow.h"
#include "./ui_MainWindow.h"

#include <QCloseEvent>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>

/******************************************************************************/

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), _ui(new Ui::MainWindow)
{
    _ui->setupUi(this);

    // Restore the window sizes
    QSettings settings;
    restoreGeometry(settings.value("UI/geometry").toByteArray());
    restoreState(settings.value("UI/windowState").toByteArray());
    _ui->verticalSplitter->setStretchFactor(0, 1);
    _ui->verticalSplitter->setStretchFactor(1, 2);
    _ui->verticalSplitter->restoreState(settings.value("UI/verticalSplitter").toByteArray());
    _ui->horizontalSplitter->setStretchFactor(0, 1);
    _ui->horizontalSplitter->setStretchFactor(1, 4);
    _ui->horizontalSplitter->restoreState(settings.value("UI/horizontalSplitter").toByteArray());

    // Sort the file names aphabetically
    _ui->fileTreeWidget->sortByColumn(0, Qt::SortOrder::AscendingOrder);
    // TODO: sort folders first?

    _project = nullptr;
    projectNew();
}

MainWindow::~MainWindow()
{
    delete _ui;
}

/******************************************************************************/

static QString lastPath()
{
    return QSettings().value("lastPath", QDir::homePath()).toString();
}

static void setLastPath(const QString &path)
{
    const QString canonicalPath = QFileInfo(path).canonicalPath();
    if (!canonicalPath.isEmpty() && lastPath() != canonicalPath)
        QSettings().setValue("lastPath", canonicalPath);
}

/******************************************************************************/

void MainWindow::projectNew()
{
    if (_project)
        _project->deleteLater();

    // Create a new project
    _project = new Project(this);
    connect(_project, &Project::modifiedChanged, this, &MainWindow::projectModifiedChanged);
    connect(_project, &Project::fileAdded,       this, &MainWindow::projectFileAdded);
    connect(_project, &Project::fileRemoved,     this, &MainWindow::projectFileRemoved);

    // Reset the UI
    _ui->actionSave->setEnabled(false);
    _ui->fileTreeWidget->clear();
    _ui->fileRemoveButton->setEnabled(false);
    setWindowTitle(tr("Lila"));
}

void MainWindow::projectOpen()
{
    if (!projectPromptSave())
        return;
    const QString filePath = QFileDialog::getOpenFileName(this, tr("Open file"), lastPath(), tr("Lila project files (*.lila)"));
    if (filePath.isEmpty())
        return;
    projectNew();
    _project->open(filePath);
    setLastPath(filePath);
    setWindowTitle(tr("%1 - Lila").arg(QFileInfo(filePath).fileName()));
}

bool MainWindow::projectSaveAs()
{
    const QString filePath = QFileDialog::getSaveFileName(this, tr("Save file"), lastPath(), tr("Lila project files (*.lila)"));
    if (filePath.isEmpty())
        return false;
    if (!_project->saveAs(filePath))
        return false;
    setLastPath(filePath);
    setWindowTitle(tr("%1 - Lila").arg(QFileInfo(filePath).fileName()));
    return true;
}

bool MainWindow::projectPromptSave()
{
    if (!_project->modified())
        return true;
    const auto ret = QMessageBox::question(this,
                                           tr("Save current file?"),
                                           tr("The current file was modified, do you want to save it before proceeding?"),
                                           QMessageBox::StandardButtons(QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No | QMessageBox::StandardButton::Cancel),
                                           QMessageBox::StandardButton::Cancel);
    if (ret == QMessageBox::StandardButton::Cancel)
        return false;
    if (ret == QMessageBox::StandardButton::Yes)
        if (!_project->save())
            return projectSaveAs();
    return true;
}

/******************************************************************************/

void MainWindow::projectModifiedChanged(bool modified)
{
    _ui->actionSave->setEnabled(modified);
}

/******************************************************************************/

static QTreeWidgetItem *findTreeNode(QTreeWidget *tree, const QString &path)
{
    // Iterate over all tree items until a match is found
    for (QTreeWidgetItemIterator it(tree); *it; ++it)
        if ((*it)->data(0, Qt::UserRole).toString() == path)
            return *it;
    return nullptr;
}

static QTreeWidgetItem *createTreeNode(QTreeWidget *tree, const QString &path)
{
    // Create a new node
    const QFileInfo fi(path);
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, fi.fileName());
    item->setData(0, Qt::UserRole, path);
    if (fi.isFile())
        item->setIcon(0, QIcon(":/resources/icons/text-x-generic.svg"));
    else if (fi.isDir())
        item->setIcon(0, QIcon(":/resources/icons/folder.svg"));
    return item;
}

static QTreeWidgetItem *findOrCreateTreeNode(QTreeWidget *tree, const QString &path)
{
    // Check if a node already exists
    QTreeWidgetItem *item = findTreeNode(tree, path);
    if (item)
        return item;
    return createTreeNode(tree, path);
}

void MainWindow::projectFileAdded(QFileInfo fi)
{
    // Recursively create nodes
    QTreeWidgetItem *newItem = createTreeNode(_ui->fileTreeWidget, fi.canonicalFilePath());
    QTreeWidgetItem *item = newItem;
    for (fi = QFileInfo(fi.canonicalPath()); !fi.isRoot(); fi = QFileInfo(fi.canonicalPath()))
    {
        QTreeWidgetItem *parent = findOrCreateTreeNode(_ui->fileTreeWidget, fi.canonicalFilePath());
        parent->addChild(item);
        item = parent;
    }
    _ui->fileTreeWidget->invisibleRootItem()->addChild(item);

    // Expand parent folders (has to be done separately)
    for (item = newItem; item; item = item->parent())
        item->setExpanded(true);
}

void MainWindow::projectFileRemoved(QFileInfo fi)
{
    // Find the file in the list
    QTreeWidgetItem *item = findTreeNode(_ui->fileTreeWidget, fi.canonicalFilePath());
    if (item)
    {
        // Remove the file
        QTreeWidgetItem *parent = item->parent();
        if (parent)
            parent->removeChild(item);
        delete item;

        // Remove the parent folders if they are empty
        while (parent && parent->childCount() == 0)
        {
           item = parent;
           parent = item->parent();
           if (parent)
               parent->removeChild(item);
           delete item;
        }
    }
}

/******************************************************************************/

void MainWindow::on_fileTreeWidget_itemSelectionChanged()
{
    _ui->fileRemoveButton->setEnabled(_ui->fileTreeWidget->selectedItems().count() != 0);
}

void MainWindow::on_fileAddButton_clicked()
{
    const QStringList filePaths = QFileDialog::getOpenFileNames(this, tr("Select file(s) to add"), lastPath(), tr("VHDL files (*.vhd *.vhdl);;Verilog files (*.v)"));
    for (const QString &filePath : filePaths)
    {
        _project->addFile(filePath);
        setLastPath(filePath);
    }
}

void MainWindow::on_fileRemoveButton_clicked()
{
    for (const auto item : _ui->fileTreeWidget->selectedItems())
        _project->removeFile(item->data(0, Qt::UserRole).toString());
}

/******************************************************************************/

void MainWindow::on_actionNew_triggered()
{
    if (!projectPromptSave())
        return;
    projectNew();
}

void MainWindow::on_actionOpen_triggered()
{
    projectOpen();
}

void MainWindow::on_actionSave_triggered()
{
    if (!_project->save())
        projectSaveAs();
}

void MainWindow::on_actionSaveAs_triggered()
{
    projectSaveAs();
}

void MainWindow::on_actionExit_triggered()
{
    close();
}

/******************************************************************************/

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->ignore();
    if (projectPromptSave())
        event->accept();

    // Save the window sizes
    QSettings settings;
    settings.setValue("UI/verticalSplitter",   _ui->verticalSplitter->saveState());
    settings.setValue("UI/horizontalSplitter", _ui->horizontalSplitter->saveState());
    settings.setValue("UI/geometry",           saveGeometry());
    settings.setValue("UI/windowState",        saveState());
}
