#include "Logger.h"

#include <QDateTime>

/******************************************************************************/

Logger *Logger::_instance = nullptr;

Logger::Logger() : QObject(nullptr) { }

Logger *Logger::instance()
{
    if (_instance == nullptr)
        _instance = new Logger;
    return _instance;
}

/******************************************************************************/

void Logger::log(LogLevel logLevel, const QString &message)
{
    // Only broadcast for now, but we could also write it to a log file…
    const QString m = QString("[%1] %2").arg(QDateTime::currentDateTime().toString(Qt::ISODateWithMs)).arg(message);
    emit logReceived(logLevel, m);
}

/******************************************************************************/

void Logger::error(const QString &message)
{
    instance()->log(LogLevel::Error, message);
}

void Logger::warning(const QString &message)
{
    instance()->log(LogLevel::Warning, message);
}

void Logger::info(const QString &message)
{
    instance()->log(LogLevel::Info, message);
}

void Logger::debug(const QString &message)
{
    instance()->log(LogLevel::Debug, message);
}
