/* Lambila | Design.h
 * Copyright (c) 2025 L. Sartory
 * SPDX-License-Identifier: MIT
 */

/******************************************************************************/

#ifndef DESIGN_H
#define DESIGN_H

#include "Logger.h"

#include <QMultiHash>

/******************************************************************************/

typedef QString Signal;
typedef QPair<QString, QString> Constant;

class Architecture {
protected:
    QString _name;
    QHash<QString, Constant *> _constants;
    QHash<QString, Signal *> _signals;

public:
    ~Architecture()
    {
        reset();
    }

    void reset()
    {
        _name = "";
        for (auto constant : _constants)
            delete constant;
        _constants.clear();
        for (auto signal : _signals)
            delete signal;
        _signals.clear();
    }

    const QString &name()
    {
        return _name;
    }
    void setName(const QString &name)
    {
        _name = name.trimmed();
    }

    Constant *constant(QString name)
    {
        return _constants.value(name.trimmed(), nullptr);
    }
    const QHash<QString, Constant *> &getConstants()
    {
        return _constants;
    }
    void addConstant(const QString &name, const QString &type, const QString &value)
    {
        Logger::debug(QString("%1 add constant: %2 | %3 | %4").arg(_name).arg(name.trimmed()).arg(type.trimmed()).arg(value.trimmed()));
        _constants.insert(name.trimmed(), new Constant(type.trimmed(), value.trimmed()));
    }

    Signal *signal(QString name)
    {
        return _signals.value(name.trimmed(), nullptr);
    }
    const QHash<QString, Signal *> &getSignals()
    {
        return _signals;
    }
    void addSignal(const QString &name, const QString &type)
    {
        Logger::debug(QString("%1 add signal: %2 | %3").arg(_name).arg(name.trimmed()).arg(type.trimmed()));
        _signals.insert(name.trimmed(), new Signal(type.trimmed()));
    }
};

/******************************************************************************/

typedef QString Use;
typedef QPair<QString, QString> Port;

class Entity {
protected:
    QString _name;
    QMultiHash<QString, Use> _uses;
    QHash<QString, Port *> _ports;
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
        for (auto port : _ports)
            delete port;
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
        _name = name.trimmed();
    }

    void addUse(const QString &library, const QString &use)
    {
        _uses.insert(library.trimmed(), use.trimmed());
    }

    Port *port(QString name)
    {
        return _ports.value(name.trimmed(), nullptr);
    }
    const QHash<QString, Port *> &getPorts()
    {
        return _ports;
    }
    void addPort(const QString &name, const QString &direction, const QString &type)
    {
        Logger::debug(QString("%1 add port: %2 | %3 | %4").arg(_name).arg(name.trimmed()).arg(direction.trimmed()).arg(type.trimmed()));
        _ports.insert(name.trimmed(), new Port(direction.trimmed(), type.trimmed()));
    }

    Architecture *architecture(QString name)
    {
        return _architectures.value(name.trimmed(), nullptr);
    }
    const QHash<QString, Architecture *> &getArchitectures()
    {
        return _architectures;
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
    const QHash<QString, Entity *> &getEntities()
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
