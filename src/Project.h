#ifndef PROJECT_H
#define PROJECT_H

#include "Design.h"

#include <QFileInfo>
#include <QProgressDialog>
#include <QThread>

class ProjectParserThread : public QThread
{
    Q_OBJECT

protected:
    QList<QFileInfo> _files;
    Design *_design;

public:
    ProjectParserThread(QList<QFileInfo> files, Design *design, QObject *parent = nullptr);

protected:
    void run() override;

signals:
    void progressChanged(int progress);
};

class Project : public QObject
{
    Q_OBJECT

private:
    static const QString _lambilaVersion;
    QFileInfo _projectFile;
    bool _modified;
    QList<QFileInfo> _files;
    Design *_design;
    ProjectParserThread *_thread;
    QProgressDialog *_progressDialog;

public:
    Project(QObject *parent = nullptr);
    ~Project();

    static QString version();

protected:
    void setModified(bool);

public:
    QFileInfo projectFile();
    bool modified();

    bool open(const QString &filePath);
    bool saveAs(const QString &filePath);
    bool save();

    bool addFile(const QString &filePath);
    bool removeFile(const QString &filePath);

    void refresh();

signals:
    void modifiedChanged(bool modified);
    void fileAdded(QFileInfo fi);
    void fileRemoved(QFileInfo fi);
};
#endif // PROJECT_H
