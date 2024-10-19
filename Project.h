#ifndef PROJECT_H
#define PROJECT_H

#include <QFileInfo>

class Project : public QObject
{
    Q_OBJECT

private:
    static const QString _lambilaVersion;
    QFileInfo _projectFile;
    bool _modified;
    QList<QFileInfo> _files;

public:
    Project(QObject *parent = nullptr);
    ~Project();

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

    bool refresh();

signals:
    void modifiedChanged(bool modified);
    void fileAdded(QFileInfo fi);
    void fileRemoved(QFileInfo fi);
};
#endif // PROJECT_H
