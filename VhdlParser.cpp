#include "VhdlParser.h"

#include <QApplication>
#include <QMessageBox>
#include <QRegularExpression>

/******************************************************************************/

VhdlParser::VhdlParser(const QFileInfo &sourceFile, QObject *parent) : QObject(parent)
{
    _sourceFile = sourceFile;
}

VhdlParser::~VhdlParser()
{
}

/******************************************************************************/

#include <QDebug>
bool VhdlParser::parse()
{
    // Open the source file
    QFile file(_sourceFile.canonicalFilePath());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QMessageBox::critical(QApplication::activeWindow(), tr("Open failed"), tr("Failed to open file: %1").arg(file.errorString()));
        return false;
    }

    // Parse the file
    while (!file.atEnd())
    {
        const QString line = QString(file.readLine());
        for (const auto token : line.split(QRegularExpression("(?=[;\\s])|(?<=[;\\s])"), Qt::SkipEmptyParts))
        //for (const auto token : line.split(QRegularExpression("[ ;\r\n]"), Qt::SkipEmptyParts))
        {
            // Skip whitespaces
            if (token.trimmed().isEmpty())
                continue;
            // Skip comments
            if (token.startsWith("--"))
                break;
            qDebug() << token;
        }
    }

    return true; // TODO
}
