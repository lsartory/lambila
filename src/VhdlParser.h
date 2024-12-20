#ifndef VHDLPARSER_H
#define VHDLPARSER_H

#include "Design.h"

#include <QFileInfo>

class VhdlParser : public QObject
{
    Q_OBJECT

protected:
    enum class State;
    enum class Target;
    class Token;

    QFileInfo _sourceFile;
    Design *_design;

public:
    VhdlParser(const QFileInfo &sourceFile, Design *design, QObject *parent = nullptr);

    bool parse();
};
#endif // VHDLPARSER_H
