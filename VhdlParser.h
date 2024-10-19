#ifndef VHDLPARSER_H
#define VHDLPARSER_H

#include <QFileInfo>
#include <QMultiHash>

class VhdlParser : public QObject
{
    Q_OBJECT

protected:
    enum class State;
    class Token;

    QFileInfo _sourceFile;
    QMultiHash<QString, QString> _uses;

public:
    VhdlParser(const QFileInfo &sourceFile, QObject *parent = nullptr);

    bool parse();
};
#endif // VHDLPARSER_H
