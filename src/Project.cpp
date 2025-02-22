/* Lambila | Project.cpp
 * Copyright (c) 2025 L. Sartory
 * SPDX-License-Identifier: MIT
 */

/******************************************************************************/

#include "Logger.h"
#include "Project.h"
#include "VhdlParser.h"

#include <QApplication>
#include <QDir>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QSaveFile>

/******************************************************************************/

const QString Project::_lambilaVersion = "1.0";

/******************************************************************************/

Project::Project(QObject *parent) : QObject(parent)
{
    _modified = false;
    _design = nullptr;
    _thread = nullptr;
    _progressDialog = nullptr;

}

Project::~Project()
{
    delete _design;
    if (_thread)
        _thread->wait();
    delete _thread;
    delete _progressDialog;
}

QString Project::version()
{
    return _lambilaVersion;
}

/******************************************************************************/

QFileInfo Project::projectFile()
{
    return _projectFile;
}

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
    // Open the project file and parse it
    QFile file(filePath);
    Logger::info(tr("Opening project %1").arg(filePath));
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        Logger::error(tr("Failed to open file: %1").arg(file.errorString()));
        return false;
    }
    QJsonParseError e;
    const QJsonDocument jdoc = QJsonDocument::fromJson(file.readAll(), &e);
    if (e.error != QJsonParseError::ParseError::NoError)
    {
        Logger::error(tr("Failed to parse file: %1").arg(e.errorString()));
        return false;
    }
    const QJsonObject jobj = jdoc.object();

    // Set the project file since it appears to be valid
    _projectFile.setFile(filePath);
    const QDir targetDir(_projectFile.absolutePath());

    // Load data
    if (jobj["_lambilaVersion"].toString() != _lambilaVersion)
        Logger::warning(tr("This file was created by a different Lambila version. Current version: %1, file version: %2").arg(_lambilaVersion).arg(jobj["_lambilaVersion"].toString()));
    for (const auto &item : jobj["fileList"].toArray())
        addFile(targetDir.filePath(item.toString()));

    // Mark the project as not modified
    setModified(false);
    return true;
}

bool Project::saveAs(const QString &filePath)
{
    // Set the new project file
    _projectFile.setFile(filePath);
    if (_projectFile.suffix().isEmpty())
        _projectFile.setFile(filePath + ".lila");
    const QDir targetDir(_projectFile.absolutePath());

    // Serialize the settings into JSON
    QJsonObject jobj;
    jobj["_lambilaVersion"] = _lambilaVersion;
    QStringList files;
    for (const QFileInfo &file : _files)
        files.append(targetDir.relativeFilePath(file.canonicalFilePath()));
    jobj["fileList"] = QJsonArray::fromStringList(files);

    // Save the file
    QSaveFile file(_projectFile.absoluteFilePath());
    file.setDirectWriteFallback(true);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        Logger::error(tr("Save failed - Open file: %1").arg(file.errorString()));
        return false;
    }
    if (file.write(QJsonDocument(jobj).toJson()) == -1)
    {
        Logger::error(tr("Save failed - Write: %1").arg(file.errorString()));
        return false;
    }
    if (!file.commit())
    {
        Logger::error(tr("Save failed - Commit: %1").arg(file.errorString()));
        return false;
    }

    // Mark the project as not modified anymore
    setModified(false);
    return true;
}

bool Project::save()
{
    // If a file path is known, reuse it
    if (_projectFile.isWritable())
        return saveAs(_projectFile.canonicalFilePath());
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

/******************************************************************************/

ProjectParserThread::ProjectParserThread(QList<QFileInfo> files, Design *design, QObject *parent) : QThread(parent)
{
    _files = files;
    _design = design;
}

void ProjectParserThread::run()
{
    // Parse all files sequentially
    int progress = 0;
    for (auto file : _files)
    {
        if (!VhdlParser(file, _design).parse())
            break;
        emit progressChanged(++progress);
    }
}

void Project::refresh()
{
    // Create a new design
    delete _design;
    _design = new Design;

    // Create a progress dialog to block the UI while refreshing
    _progressDialog = new QProgressDialog(tr("Refreshing..."), "", 0, _files.count());
    _progressDialog->setCancelButton(nullptr);
    _progressDialog->setModal(true);
    _progressDialog->show();

    // Use a thread to parse all files
    _thread = new ProjectParserThread(_files, _design, this);
    connect(_thread, &ProjectParserThread::progressChanged, _progressDialog, &QProgressDialog::setValue);
    connect(_thread, &QThread::finished, [=] {
        // Clean up
        _thread->deleteLater();
        _thread = nullptr;
        _progressDialog->deleteLater();
        _progressDialog = nullptr;

        // TODO: build hierarchy
        Logger::debug("Found entities:");
        for (auto entity : _design->getEntities())
            Logger::debug(entity->name());
    });
    _thread->start();
}
