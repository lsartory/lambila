#ifndef VHDLPARSER_H
#define VHDLPARSER_H

#include <QFileInfo>

class VhdlParser : public QObject
{
    Q_OBJECT

protected:
    QFileInfo _sourceFile;

public:
    VhdlParser(const QFileInfo &sourceFile, QObject *parent = nullptr);
    ~VhdlParser();

    bool parse();
};
#endif // VHDLPARSER_H
