#ifndef LOGGER_H
#define LOGGER_H

#include <QObject>

class Logger : public QObject
{
    Q_OBJECT

public:
    enum class LogLevel {
        Error,
        Warning,
        Info,
        Debug,
        Trace
    };

private:
    LogLevel _verbosity;

    static Logger *_instance;
    Logger();

    void log(LogLevel logLevel, const QString &message);

public:
    static Logger *instance();

    static LogLevel verbosity();
    static void setVerbosity(LogLevel logLevel);

    static void error(const QString &message);
    static void warning(const QString &message);
    static void info(const QString &message);
    static void debug(const QString &message);
    static void trace(const QString &message);

signals:
    void logReceived(LogLevel logLevel, const QString &message);
};
#endif // LOGGER_H
