/* Lambila | Logger.cpp
 * Copyright (c) 2025 L. Sartory
 * SPDX-License-Identifier: MIT
 */

/******************************************************************************/

#include "Logger.h"

#include <QDateTime>

/******************************************************************************/

Logger *Logger::_instance = nullptr;

Logger::Logger() : QObject(nullptr)
{
    //_verbosity = LogLevel::Info;
    _verbosity = LogLevel::Trace; // TODO: make this configurable through a settings window
}

/******************************************************************************/

void Logger::log(LogLevel logLevel, const QString &message)
{
    // Ignore messages with levels higher than the currently set verbosity
    if (logLevel > _verbosity)
        return;

    // Only broadcast for now, but we could also write it to a log file…
    const QString m = QString("[%1] %2").arg(QDateTime::currentDateTime().toString(Qt::ISODateWithMs)).arg(message);
    emit logReceived(logLevel, m);
}

/******************************************************************************/

Logger *Logger::instance()
{
    if (_instance == nullptr)
        _instance = new Logger;
    return _instance;
}

/******************************************************************************/

Logger::LogLevel Logger::verbosity()
{
    return instance()->_verbosity;
}

void Logger::setVerbosity(Logger::LogLevel logLevel)
{
    instance()->_verbosity = logLevel;
}

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

void Logger::trace(const QString &message)
{
    instance()->log(LogLevel::Trace, message);
}
