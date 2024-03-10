#include "Project.h"

#include <QApplication>
#include <QDir>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QMessageBox>
#include <QSaveFile>

/******************************************************************************/

const QString Project::_lilaVersion = "1.0";

/******************************************************************************/

Project::Project(QObject *parent) : QObject(parent)
{
    _modified = false;
    _projectFile = nullptr;
}

Project::~Project()
{
    delete _projectFile;
}

/******************************************************************************/

bool Project::modified()
{
    return _modified;
}

void Project::setModified(bool modified)
{
    if (modified == _modified)
        return;
    _modified = modified;
    emit modifiedChanged(_modified);
}

/******************************************************************************/

bool Project::open(const QString &filePath)
{
    // Set the new project file
    delete _projectFile;
    _projectFile = new QFileInfo(filePath);
    const QDir targetDir(_projectFile->canonicalPath());

    // Open the file and parse it
    QFile file(_projectFile->canonicalFilePath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(QApplication::activeWindow(), tr("Open failed"), tr("Failed to open file: %1").arg(file.errorString()));
        return false;
    }
    QJsonParseError e;
    const QJsonDocument jdoc = QJsonDocument::fromJson(file.readAll(), &e);
    if (e.error != QJsonParseError::ParseError::NoError)
    {
        QMessageBox::critical(QApplication::activeWindow(), tr("Open failed"), tr("Failed to parse file: %1").arg(e.errorString()));
        return false;
    }
    const QJsonObject jobj = jdoc.object();

    // Load data
    if (jobj["_lilaVersion"].toString() != _lilaVersion)
        QMessageBox::warning(QApplication::activeWindow(), tr("Version mismatch"), tr("This file was created by a different Lila version.\n\nCurrent Lila version: %1\nFile version: %2").arg(_lilaVersion).arg(jobj["_lilaVersion"].toString()));
    for (const auto &item : jobj["fileList"].toArray())
        addFile(targetDir.filePath(item.toString()));

    // Mark the project as not modified
    setModified(false);
    return true;
}

bool Project::saveAs(const QString &filePath)
{
    // Set the new project file
    delete _projectFile;
    _projectFile = new QFileInfo(filePath);
    if (_projectFile->suffix().isEmpty())
        _projectFile->setFile(filePath + ".lila");
    const QDir targetDir(_projectFile->canonicalPath());

    // Serialize the settings into JSON
    QJsonObject jobj;
    jobj["_lilaVersion"] = _lilaVersion;
    QStringList files;
    for (const QFileInfo &file : _files)
        files.append(targetDir.relativeFilePath(file.canonicalFilePath()));
    jobj["fileList"] = QJsonArray::fromStringList(files);

    // Save the file
    QSaveFile file(_projectFile->absoluteFilePath());
    file.setDirectWriteFallback(true);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        QMessageBox::critical(QApplication::activeWindow(), tr("Save failed"), tr("Failed to open file: %1").arg(file.errorString()));
        return false;
    }
    if (file.write(QJsonDocument(jobj).toJson()) == -1)
    {
        QMessageBox::critical(QApplication::activeWindow(), tr("Save failed"), tr("Failed to write file: %1").arg(file.errorString()));
        return false;
    }
    if (!file.commit())
    {
        QMessageBox::critical(QApplication::activeWindow(), tr("Save failed"), tr("Failed to save file: %1").arg(file.errorString()));
        return false;
    }

    // Mark the project as not modified anymore
    setModified(false);
    return true;
}

bool Project::save()
{
    // If a file path is known, reuse it
    if (_projectFile)
        return saveAs(_projectFile->canonicalFilePath());
    return false;
}

/******************************************************************************/

bool Project::addFile(const QString &filePath)
{
    // Ensure the file does not exist in the list already
    for (const QFileInfo &f : _files)
        if (f.canonicalFilePath() == filePath)
            return false;

    // Ensure the file exists
    const QFileInfo fi(filePath);
    if (!fi.exists())
        return false;

    // Add the file to the list
    _files.append(fi);
    emit fileAdded(fi);
    setModified(true);
    return true;
}

bool Project::removeFile(const QString &filePath)
{
    // Find the file in the list and remove it
    for (int i = 0; i < _files.count(); ++i)
    {
        if (_files.at(i).canonicalFilePath() == filePath)
        {
            emit fileRemoved(_files.takeAt(i));
            setModified(true);
            return true;
        }
    }
    return false;
}
