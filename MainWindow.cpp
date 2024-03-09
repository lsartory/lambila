#include "MainWindow.h"
#include "./ui_MainWindow.h"

#include <QFileDialog>
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
    const QString lastPath = QSettings().value("lastPath", QDir::homePath()).toString();
    const QStringList filePaths = QFileDialog::getOpenFileNames(this, tr("Select file(s) to open"), lastPath, tr("VHDL files (*.vhd *.vhdl);;Verilog files (*.v)"));
    for (const QString filePath : filePaths)
    {
        _project->addFile(filePath);
        const QString canonicalPath = QFileInfo(filePath).canonicalPath();
        if (!canonicalPath.isEmpty() && lastPath != canonicalPath)
            QSettings().setValue("lastPath", canonicalPath);
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
    // TODO: save?
    projectNew();
}

void MainWindow::on_actionSave_triggered()
{
    _project->save();
}

void MainWindow::on_actionSaveAs_triggered()
{
    const QString lastPath = QSettings().value("lastPath", QDir::homePath()).toString();
    const QString filePath = QFileDialog::getSaveFileName(this, tr("Save to file"), lastPath, tr("Lila project files (*.lila)"));
    if (filePath.isEmpty())
        return;
    _project->saveAs(filePath);
    const QFileInfo fi(filePath);
    const QString canonicalPath = fi.canonicalPath();
    if (!canonicalPath.isEmpty() && lastPath != canonicalPath)
        QSettings().setValue("lastPath", canonicalPath);
}

void MainWindow::on_actionExit_triggered()
{
    // TODO: save?
    close();
}
