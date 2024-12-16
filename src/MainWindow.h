#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Logger.h"
#include "Project.h"

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

private:
    Ui::MainWindow *_ui;
    Project *_project;

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void projectNew();
    void projectOpen(QString filePath = QString());
    bool projectSaveAs();
    bool projectPromptSave();

    virtual void closeEvent(QCloseEvent *event);

private slots:
    void projectModifiedChanged(bool modified);
    void projectFileAdded(QFileInfo fi);
    void projectFileRemoved(QFileInfo fi);

    void on_fileTreeWidget_itemSelectionChanged();

    void on_fileAddButton_clicked();
    void on_fileRemoveButton_clicked();

    void on_refreshButton_clicked();

    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();
    void on_actionExit_triggered();

    void logReceived(Logger::LogLevel logLevel, const QString &message);
};
#endif // MAINWINDOW_H
