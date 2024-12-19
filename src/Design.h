#ifndef DESIGN_H
#define DESIGN_H

#include "Logger.h"

#include <QMultiHash>

/******************************************************************************/

class Architecture {
protected:
    QString _name;
    QHash<QString, QString> _signals;

public:
    const QString &name()
    {
        return _name;
    }
    void setName(const QString &name)
    {
        _name = name;
    }
    void addSignal(const QString &name, const QString &type)
    {
        Logger::debug(QString("%1 add signal: %2 | %3").arg(_name).arg(name).arg(type));
        _signals.insert(name, type);
    }
};

/******************************************************************************/

typedef QPair<QString, QString> Port;

class Entity {
protected:
    QString _name;
    QMultiHash<QString, QString> _uses;
    QHash<QString, Port> _ports;
    QHash<QString, Architecture *> _architectures;

public:
    ~Entity()
    {
        reset();
    }

    void reset()
    {
        _name = "";
        _uses.clear();
        _ports.clear();
        for (auto architecture : _architectures)
            delete architecture;
        _architectures.clear();
    }

    const QString &name()
    {
        return _name;
    }
    void setName(const QString &name)
    {
        _name = name;
    }
    void addUse(const QString &library, const QString &use)
    {
        _uses.insert(library, use);
    }
    void addPort(const QString &name, const QString &direction, const QString &type)
    {
        Logger::debug(QString("%1 add port: %2 | %3 | %4").arg(_name).arg(name).arg(direction).arg(type));
        _ports.insert(name, Port(direction, type));
    }
    void addArchitecture(Architecture *architecture)
    {
        Logger::debug(QString("%1 add architecture: %2").arg(_name).arg(architecture->name()));
        _architectures.insert(architecture->name(), architecture);
    }
};

/******************************************************************************/

class Design {
protected:
    QHash<QString, Entity *> _entities;

public:
    ~Design()
    {
        for (auto entity : _entities)
            delete entity;
    }

    Entity *entity(QString name)
    {
        return _entities.value(name, nullptr);
    }
    const QHash<QString, Entity *> &entities()
    {
        return _entities;
    }
    void addEntity(Entity *entity)
    {
        Logger::debug(QString("add entity: %1").arg(entity->name()));
        _entities.insert(entity->name(), entity);
    }
};

/******************************************************************************/

#endif // DESIGN_H
