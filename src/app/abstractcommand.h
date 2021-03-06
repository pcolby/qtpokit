// SPDX-FileCopyrightText: 2022 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: LGPL-3.0-or-later

#ifndef QTPOKIT_ABSTRACTCOMMAND_H
#define QTPOKIT_ABSTRACTCOMMAND_H

#include <QBluetoothDeviceInfo>
#include <QCommandLineParser>
#include <QLoggingCategory>
#include <QObject>

class PokitDiscoveryAgent;

class AbstractCommand : public QObject
{
public:
    enum class OutputFormat {
        Csv,  ///< RFC 4180 compliant CSV text.
        Json, ///< RFC 8259 compliant JSON text.
        Text, ///< Plain unstructured text.
    };

    explicit AbstractCommand(QObject * const parent);

    virtual QStringList requiredOptions(const QCommandLineParser &parser) const;
    virtual QStringList supportedOptions(const QCommandLineParser &parser) const;

    static QString escapeCsvField(const QString &field);
    static quint32 parseMicroValue(const QString &value, const QString &unit,
                                   const quint32 sensibleMinimum=0);
    static quint32 parseMilliValue(const QString &value, const QString &unit,
                                   const quint32 sensibleMinimum=0);
    static quint32 parseWholeValue(const QString &value, const QString &unit);

public slots:
    virtual QStringList processOptions(const QCommandLineParser &parser);
    virtual bool start() = 0;

protected:
    PokitDiscoveryAgent * discoveryAgent; ///< Agent for Pokit device descovery.
    OutputFormat format; ///< Selected output format.
    static Q_LOGGING_CATEGORY(lc, "pokit.ui.command", QtInfoMsg); ///< Logging category for UI commands.

protected slots:
    QString deviceToScanFor; ///< Device (if any) that were passed to processOptions().
    virtual void deviceDiscovered(const QBluetoothDeviceInfo &info) = 0;
    virtual void deviceDiscoveryFinished() = 0;

    friend class TestAbstractCommand;
};

#endif // QTPOKIT_ABSTRACTCOMMAND_H
