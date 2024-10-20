#include "Logger.h"
#include "VhdlParser.h"

#include <QApplication>
#include <QRegularExpression>
#include <QStack>

/******************************************************************************/

VhdlParser::VhdlParser(const QFileInfo &sourceFile, QObject *parent) : QObject(parent)
{
    _sourceFile = sourceFile;
}

/******************************************************************************/

enum class VhdlParser::State {
    Base = 0x1000,
    Library,
    Use,

    Entity = 0x2000,
    EntityBody,
    EntityGeneric,
    EntityPort,

    Architecture = 0x3000,

    ExpectIs = 0xA000,
    ExpectOpeningParenthesis,
    ExpectClosingParenthesis,
    ExpectSemicolon,

    SkipToClosingParenthesis = 0xB000,
    SkipToSemicolon
};

/******************************************************************************/

class VhdlParser::Token : public QString {
public:
    Token(const QString &str) : QString(str) { }

    bool is(const QString &str) const
    {
        return compare(str, Qt::CaseInsensitive) == 0;
    }
    bool is(const char *str) const
    {
        return compare(str, Qt::CaseInsensitive) == 0;
    }
    bool is(const char c) const
    {
        return compare(c, Qt::CaseInsensitive) == 0;
    }
    bool matches(const QString &re) const
    {
        return contains(QRegularExpression(QString("^%1$").arg(re)));
    }

    bool isCommentStart() const
    {
        return startsWith("--");
    }

    bool isWhitespace() const
    {
        return trimmed().isEmpty();
    }
};

/******************************************************************************/

bool VhdlParser::parse()
{
    QStack<State> state;
    state.push(State::Base);

    QString currentEntity;

    // Open the source file
    const QString filePath = _sourceFile.canonicalFilePath();
    Logger::info(tr("Parsing %1").arg(filePath));
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        Logger::error(tr("Failed to open file: %1").arg(file.errorString()));
        return false;
    }

    // Parse the file
    unsigned int lineNumber = 0;
    while (!file.atEnd())
    {
        lineNumber += 1;
        const QString line = QString(file.readLine());
        for (const Token token : line.split(QRegularExpression("(?=[\\(\\):;\\s])|(?<=[\\(\\):;\\s])"), Qt::SkipEmptyParts))
        {
            // Skip whitespaces
            if (token.isWhitespace())
                continue;
            // Skip comments
            if (token.isCommentStart())
                break;

            Logger::debug(tr("state = 0x%1; token = %2").arg(static_cast<unsigned int>(state.top()), 4, 16, QChar('0')).arg(token)); // TODO: debug only

            // Check token depending on the current state
            switch (state.top()) {

            case State::Base:
                if (token.is("library"))
                    state.push(State::Library);
                else if (token.is("use"))
                    state.push(State::Use);
                else if (token.is("entity"))
                    state.push(State::Entity);
                else if (token.is("architecture"))
                    state.push(State::Architecture);
                else
                    goto unexpected;
                break;

            case State::Library:
                // We probably don't really need to track libraries
                if (!token.is(';'))
                    state.top() = State::SkipToSemicolon;
                else
                    goto unexpected;
                break;

            case State::Use:
                // We accept pretty much anything for now
                if (token.matches("\\w+\\..*"))
                {
                    _uses.insert(token.section('.', 0, 0), token.section('.', 1));
                    state.top() = State::ExpectSemicolon;
                }
                else
                    goto unexpected;
                break;

            case State::Entity:
                if (token.matches("\\w+"))
                {
                    currentEntity = token;
                    state.top() = State::EntityBody;
                    state.push(State::ExpectIs);
                }
                else
                    goto unexpected;
                break;
            case State::EntityBody:
                if (token.is("generic"))
                {
                    state.push(State::EntityGeneric);
                    state.push(State::ExpectOpeningParenthesis);
                }
                else if (token.is("port"))
                {
                    state.push(State::EntityPort);
                    state.push(State::ExpectOpeningParenthesis);
                }
                else if (token.is("end"))
                    state.top() = State::SkipToSemicolon;
                else
                    goto unexpected;
                break;
            case State::EntityGeneric:
                // Skip generic definitions
                state.top() = State::ExpectSemicolon;
                if (!token.is(')'))
                    state.push(State::SkipToClosingParenthesis);
                break;
            case State::EntityPort:
                // TODO: save signals
                state.top() = State::ExpectSemicolon;
                if (!token.is(')'))
                    state.push(State::SkipToClosingParenthesis);
                break;

            case State::ExpectIs:
                if (token.is("is"))
                    state.pop();
                else
                    goto unexpected;
                break;
            case State::ExpectOpeningParenthesis:
                if (token.is('('))
                    state.pop();
                else
                    goto unexpected;
                break;
            case State::ExpectClosingParenthesis:
                if (token.is(')'))
                    state.pop();
                else
                    goto unexpected;
                break;
            case State::ExpectSemicolon:
                if (token.is(';'))
                    state.pop();
                else
                    goto unexpected;
                break;

            case State::SkipToClosingParenthesis:
                if (token.is(')'))
                    state.pop();
                else if (token.is('('))
                    state.push(State::SkipToClosingParenthesis);
                break;
            case State::SkipToSemicolon:
                if (token.is(';'))
                    state.pop();
                break;

            default:
                Logger::error(tr("%1:%2 Unexpected state (0x%3)").arg(filePath).arg(lineNumber).arg(static_cast<unsigned int>(state.top()), 4, 16, QChar('0')));
                return false;
            }
            continue;

unexpected:
            Logger::error(tr("%1:%2 “%3” unexpected (state = 0x%4)").arg(filePath).arg(lineNumber).arg(token).arg(static_cast<unsigned int>(state.top()), 4, 16, QChar('0')));
            return false;
        }
    }

    if (state.top() != State::Base)
    {
        Logger::error(tr("%1 Unexpected end of file (state = 0x%2)").arg(filePath).arg(static_cast<unsigned int>(state.top()), 4, 16, QChar('0')));
        return false;
    }

    return true;
}
