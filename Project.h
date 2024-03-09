#ifndef PROJECT_H
#define PROJECT_H

#include <QFileInfo>

class Project : public QObject
{
    Q_OBJECT

private:
    QFileInfo *_projectFile;
    QList<QFileInfo> _files;

public:
    Project(QObject *parent = nullptr);
    ~Project();

    void saveAs(const QString &filePath);
    void save();

    void addFile(const QString &filePath);
    void removeFile(const QString &filePath);

signals:
    void fileAdded(QFileInfo fi);
    void fileRemoved(QFileInfo fi);
};
#endif // PROJECT_H
