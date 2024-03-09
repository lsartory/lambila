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
}

bool MainWindow::projectSaveAs()
{
    const QString filePath = QFileDialog::getSaveFileName(this, tr("Save file"), lastPath(), tr("Lila project files (*.lila)"));
    if (filePath.isEmpty())
        return false;
    if (!_project->saveAs(filePath))
        return false;
    setLastPath(filePath);
    return true;
}

bool MainWindow::projectPromptSave()
{
    if (!_project->modified())
        return true;
    if (QMessageBox::question(this, tr("Save current file?"), tr("The current file was modified, do you want to save it before proceeding?")) == QMessageBox::StandardButton::Yes)
        if (!_project->save())
            return projectSaveAs();
    return true;
}

/******************************************************************************/

void MainWindow::projectModifiedChanged(bool modified)
{
    _ui->actionSave->setEnabled(modified);
}

void MainWindow::projectFileAdded(QFileInfo fi)
{
    // Create a new entry in the file list
    QTreeWidgetItem *item = new QTreeWidgetItem();
    item->setText(0, fi.fileName());
    item->setData(0, Qt::UserRole, fi.canonicalFilePath());
    _ui->fileTreeWidget->addTopLevelItem(item);
    _ui->fileTreeWidget->sortByColumn(0, Qt::SortOrder::AscendingOrder);
}

void MainWindow::projectFileRemoved(QFileInfo fi)
{
    // Find the file in the list and remove it
    for (const auto item : _ui->fileTreeWidget->findItems("", Qt::MatchStartsWith))
    {
        if (item->data(0, Qt::UserRole).toString() == fi.canonicalFilePath())
        {
            _ui->fileTreeWidget->invisibleRootItem()->removeChild(item);
            delete item;
            break;
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
    for (const QString filePath : filePaths)
    {
        _project->addFile(filePath);
        setLastPath(filePath);
    }
}

void MainWindow::on_fileRemoveButton_clicked()
{
    for (auto item : _ui->fileTreeWidget->selectedItems())
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
    if (!projectPromptSave())
        return;
    const QString filePath = QFileDialog::getOpenFileName(this, tr("Open file"), lastPath(), tr("Lila project files (*.lila)"));
    if (filePath.isEmpty())
        return;
    projectNew();
    _project->open(filePath);
    setLastPath(filePath);
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
    if (!projectPromptSave())
        return;
    close();
}

/******************************************************************************/

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->ignore();
    if (projectPromptSave())
        event->accept();
}
