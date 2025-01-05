#include "Logger.h"
#include "VhdlParser.h"

#include <QApplication>
#include <QRegularExpression>
#include <QStack>

/******************************************************************************/

static const char WORKSPACE_NAME[] = "work";

/******************************************************************************/

VhdlParser::VhdlParser(const QFileInfo &sourceFile, Design *design, QObject *parent) : QObject(parent)
{
    _sourceFile = sourceFile;
    _design = design;
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
    EntityPortDirection,
    EntityPortType,
    EntityPortAssignment,

    Architecture = 0x3000,
    ArchitectureOf,
    ArchitectureHeader,
    ArchitectureSignal,
    ArchitectureSignalType,
    ArchitectureSignalAssignment,

    ExpectIs = 0xA000,
    ExpectOf,
    ExpectBegin,
    ExpectEnd,
    ExpectOpeningParenthesis,
    ExpectClosingParenthesis,
    ExpectSemicolon,
    ExpectColon,

    SkipToBegin = 0xB000,
    SkipToEnd,
    SkipToClosingParenthesis,
    SkipToSemicolon
};

enum class VhdlParser::Target {
    Signal,
    Constant
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

    bool isIdentifier() const
    {
        return matches("\\w+");
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
    QString errorString;
    QStack<State> state;
    state.push(State::Base);
    int parenCount = 0;

    Entity dummyEntity;
    Entity *currentEntity = &dummyEntity;
    Architecture *currentArchitecture = nullptr;
    Target target = Target::Signal;

    QString name;
    QString direction;
    QString type;
    QString value;

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

            // Display all tokens and stack length, for debugging
            Logger::trace(tr("state = 0x%1 | %2; token = %3").arg(static_cast<unsigned int>(state.top()), 4, 16, QChar('0')).arg(state.length()).arg(token));

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
                currentEntity = &dummyEntity;
                if (!token.is(';'))
                    state.top() = State::SkipToSemicolon;
                else
                    goto unexpected;
                break;

            case State::Use:
                // We accept pretty much anything for now
                if (token.matches("\\w+\\..*"))
                {
                    currentEntity->addUse(token.section('.', 0, 0), token.section('.', 1));
                    state.top() = State::ExpectSemicolon;
                }
                else
                    goto unexpected;
                break;

            /******************************************************************************/

            case State::Entity:
                if (token.matches("\\w+"))
                {
                    // Create a copy of the current entity and add it to the list
                    Entity *newEntity = new Entity;
                    *newEntity = *currentEntity;
                    currentEntity = newEntity;
                    currentEntity->setName(QString("%1.%2").arg(WORKSPACE_NAME).arg(token));
                    _design->addEntity(currentEntity);
                    dummyEntity.reset();

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
                    state.push(State::ExpectSemicolon);
                    state.push(State::EntityPort);
                    state.push(State::ExpectOpeningParenthesis);
                }
                else if (token.is("end"))
                    state.top() = State::SkipToSemicolon;
                else
                    goto unexpected;
                break;
            case State::EntityGeneric:
                // Skip generic definitions for now
                state.top() = State::ExpectSemicolon;
                if (!token.is(')'))
                    state.push(State::SkipToClosingParenthesis);
                break;
            case State::EntityPort:
                if (token.isIdentifier())
                {
                    name = token;
                    direction = "";
                    type = "";
                    state.push(State::EntityPortDirection);
                    state.push(State::ExpectColon);
                }
                else
                    goto unexpected;
                break;
            case State::EntityPortDirection:
                if (token.isIdentifier())
                {
                    direction = token;
                    parenCount = 0;
                    state.top() = State::EntityPortType;
                }
                else
                    goto unexpected;
                break;
            case State::EntityPortType:
                if (token.is(':'))
                {
                    if (type.isEmpty())
                        goto unexpected;
                    currentEntity->addPort(name, direction, type);
                    state.top() = State::EntityPortAssignment;
                }
                else if (token.is(';'))
                {
                    if (type.isEmpty())
                        goto unexpected;
                    currentEntity->addPort(name, direction, type);
                    state.pop();
                }
                else if (token.is(')'))
                {
                    if (parenCount != 0)
                    {
                        parenCount -= 1;
                        type += token;
                    }
                    else
                    {
                        currentEntity->addPort(name, direction, type);
                        state.pop();
                        state.pop();
                    }
                }
                else if (token.is('('))
                {
                    parenCount += 1;
                    type += token;
                }
                else
                {
                    if (!type.endsWith('('))
                        type += ' ';
                    type += token;
                }
                break;
            case State::EntityPortAssignment:
                // Default assignments are ignored
                if (token.is(';'))
                    state.pop();
                else if (token.is(')'))
                {
                    if (parenCount != 0)
                        parenCount -= 1;
                    else
                    {
                        state.pop();
                        state.pop();
                    }
                }
                else if (token.is('('))
                    parenCount += 1;
                break;

            /******************************************************************************/

            case State::Architecture:
                if (token.isIdentifier())
                {
                    name = token;
                    state.top() = State::ArchitectureOf;
                    state.push(State::ExpectOf);
                }
                else
                    goto unexpected;
                break;
            case State::ArchitectureOf:
                if (token.isIdentifier())
                {
                    currentArchitecture = new Architecture;
                    currentArchitecture->setName(name);
                    Entity *entity = _design->entity(QString("%1.%2").arg(WORKSPACE_NAME).arg(token));
                    if (entity != nullptr)
                    {
                        entity->addArchitecture(currentArchitecture);
                        state.top() = State::ArchitectureHeader;
                        state.push(State::ExpectIs);
                    }
                    else
                    {
                        errorString = QString("Unknown entity “%1”").arg(token);
                        goto error;
                    }
                }
                else
                    goto unexpected;
                break;
            case State::ArchitectureHeader:
                if (token.is("signal"))
                {
                    target = Target::Signal;
                    state.push(State::ArchitectureSignal);
                }
                else if (token.is("constant"))
                {
                    target = Target::Constant;
                    state.push(State::ArchitectureSignal);
                }
                else if (token.is("type"))
                {
                    // TODO: type parsing
                    errorString = "type parsing is not implemented yet";
                    goto error;
                }
                else if (token.is("function") || token.is("procedure"))
                {
                    // Ignore functions
                    state.push(State::SkipToSemicolon);
                    state.push(State::SkipToEnd);
                    state.push(State::SkipToBegin);
                }
                else if (token.is("component"))
                {
                    // Ignore components
                    state.push(State::SkipToSemicolon);
                    state.push(State::SkipToEnd);
                }
                else if (token.is("begin"))
                {
                    // TODO: architecture body
                    state.top() = State::SkipToSemicolon;
                    state.push(State::SkipToEnd);
                }
                else
                    goto unexpected;
                break;
            case State::ArchitectureSignal:
                if (token.isIdentifier())
                {
                    name = token;
                    type = "";
                    value = "";
                    state.top() = State::ArchitectureSignalType;
                    state.push(State::ExpectColon);
                }
                else
                    goto unexpected;
                break;
            case State::ArchitectureSignalType:
                if (token.is(':'))
                {
                    if (type.isEmpty() || parenCount != 0)
                        goto unexpected;
                    if (target == Target::Signal && currentArchitecture != nullptr)
                        currentArchitecture->addSignal(name, type);
                    state.top() = State::ArchitectureSignalAssignment;
                }
                else if (token.is(';'))
                {
                    if (type.isEmpty() || parenCount != 0 || target == Target::Constant)
                        goto unexpected;
                    if (currentArchitecture != nullptr)
                        currentArchitecture->addSignal(name, type);
                    state.pop();
                }
                else if (token.is(')'))
                {
                    if (parenCount != 0)
                    {
                        parenCount -= 1;
                        type += token;
                    }
                    else
                        goto unexpected;
                }
                else if (token.is('('))
                {
                    parenCount += 1;
                    type += token;
                }
                else
                {
                    if (!type.endsWith('('))
                        type += ' ';
                    type += token;
                }
                break;
            case State::ArchitectureSignalAssignment:
                // Default assignments are ignored
                if (token.is(';'))
                {
                    if (parenCount != 0)
                        goto unexpected;
                    if (target == Target::Constant)
                        currentArchitecture->addConstant(name, type, value);
                    state.pop();
                }
                else if (token.is(')'))
                {
                    value += token;
                    if (parenCount != 0)
                        parenCount -= 1;
                }
                else if (token.is('('))
                {
                    parenCount += 1;
                    value += token;
                }
                else
                {
                    if (!value.endsWith('('))
                        value += ' ';
                    value += token;
                }
                break;

            /******************************************************************************/

            case State::ExpectIs:
                if (token.is("is"))
                    state.pop();
                else
                    goto unexpected;
                break;
            case State::ExpectOf:
                if (token.is("of"))
                    state.pop();
                else
                    goto unexpected;
                break;
            case State::ExpectBegin:
                if (token.is("begin"))
                    state.pop();
                else
                    goto unexpected;
                break;
            case State::ExpectEnd:
                if (token.is("end"))
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
            case State::ExpectColon:
                if (token.is(':'))
                    state.pop();
                else
                    goto unexpected;
                break;

            /******************************************************************************/

            case State::SkipToBegin:
                if (token.is("begin"))
                    state.pop();
                break;
            case State::SkipToEnd:
                if (token.is("end"))
                    state.pop();
                else if (token.is("begin") || token.is("then") || token.is("for"))
                    state.push(State::SkipToEnd);
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

            /******************************************************************************/

            default:
                Logger::error(tr("%1:%2 Unexpected state (0x%3)").arg(filePath).arg(lineNumber).arg(static_cast<unsigned int>(state.top()), 4, 16, QChar('0')));
                return false;
            }
            continue;

unexpected:
            errorString = QString("“%1” unexpected (state = 0x%2)").arg(token).arg(static_cast<unsigned int>(state.top()), 4, 16, QChar('0'));
error:
            Logger::error(tr("%1:%2 %3").arg(filePath).arg(lineNumber).arg(errorString));
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
