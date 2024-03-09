#ifndef PROJECT_H
#define PROJECT_H

#include <QFileInfo>

class Project : public QObject
{
    Q_OBJECT

protected:
    bool _modified;
    QFileInfo *_projectFile;
    QList<QFileInfo> _files;

public:
    Project(QObject *parent = nullptr);
    ~Project();

protected:
    void setModified(bool);

public:
    void saveAs(const QString &filePath);
    void save();

    void addFile(const QString &filePath);
    void removeFile(const QString &filePath);

signals:
    void modifiedChanged(bool modified);
    void fileAdded(QFileInfo fi);
    void fileRemoved(QFileInfo fi);
};
#endif // PROJECT_H
