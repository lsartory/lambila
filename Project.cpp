#include "Project.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMessageBox>
#include <QSaveFile>

/******************************************************************************/

Project::Project(QObject *parent) : QObject(parent)
{
    _projectFile = nullptr;
}

Project::~Project()
{
    delete _projectFile;
}

/******************************************************************************/

void Project::saveAs(const QString &filePath)
{
    // Set the new project file
    delete _projectFile;
    _projectFile = new QFileInfo(filePath);
    if (_projectFile->suffix().isEmpty())
        _projectFile->setFile(filePath + ".lila");

    // Serialize the settings into JSON
    QJsonObject jobj;
    jobj["_lilaVersion"] = "1.0"; // TODO: define this somewere else
    QStringList files;
    for (const auto file : _files)
        files.append(file.canonicalFilePath()); // TODO: relative paths
    jobj["fileList"] = QJsonArray::fromStringList(files);
    QJsonDocument jdoc(jobj);

    // Save the file
    QSaveFile file(_projectFile->absoluteFilePath());
    file.setDirectWriteFallback(true);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::critical(nullptr, tr("Saving failed"), tr("Failed to open file: %1").arg(file.errorString()));
        return;
    }
    if (file.write(jdoc.toJson()) == -1)
    {
        QMessageBox::critical(nullptr, tr("Saving failed"), tr("Failed to write file: %1").arg(file.errorString()));
        return;
    }
    if (!file.commit())
    {
        QMessageBox::critical(nullptr, tr("Saving failed"), tr("Failed to save file: %1").arg(file.errorString()));
        return;
    }
}

void Project::save()
{
    // If a file path is known, reuse it
    if (_projectFile)
        saveAs(_projectFile->canonicalFilePath());
}

/******************************************************************************/

void Project::addFile(const QString &filePath)
{
    // Ensure the file does not exist in the list already
    for (const auto f : _files)
        if (f.canonicalFilePath() == filePath)
            return;

    // Ensure the file exists
    const QFileInfo fi(filePath);
    if (!fi.exists())
        return;

    // Add the file to the list
    _files.append(fi);
    emit fileAdded(fi);
}

void Project::removeFile(const QString &filePath)
{
    // Find the file in the list and remove it
    for (int i = 0; i < _files.count(); ++i)
    {
        if (_files.at(i).canonicalFilePath() == filePath)
        {
            emit fileRemoved(_files.takeAt(i));
            break;
        }
    }
}
