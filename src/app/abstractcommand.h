/*
    Copyright 2022 Paul Colby

    This file is part of QtPokit.

    QtPokit is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    QtPokit is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with QtPokit.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef QTPOKIT_ABSTRACTWORKER_H
#define QTPOKIT_ABSTRACTWORKER_H

#include <QCommandLineParser>
#include <QLoggingCategory>
#include <QObject>

class AbstractCommand : public QObject
{
public:
    explicit AbstractCommand(QObject * const parent);

    virtual QStringList requiredOptions() const;
    virtual QStringList supportedOptions() const;

public slots:
    virtual bool processOptions(const QCommandLineParser &parser);

protected:
    Q_LOGGING_CATEGORY(lc, "pokit.ui.command", QtInfoMsg);

private slots:
};

#endif // QTPOKIT_ABSTRACTWORKER_H