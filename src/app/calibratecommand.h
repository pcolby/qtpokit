// SPDX-FileCopyrightText: 2022 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "devicecommand.h"

class CalibrationService;

class CalibrateCommand : public DeviceCommand
{
public:
    explicit CalibrateCommand(QObject * const parent);

    QStringList requiredOptions(const QCommandLineParser &parser) const override;
    QStringList supportedOptions(const QCommandLineParser &parser) const override;

public slots:
    QStringList processOptions(const QCommandLineParser &parser) override;

protected:
    AbstractPokitService * getService() override;

protected slots:
    void serviceDetailsDiscovered() override;

private:
    CalibrationService * service; ///< Bluetooth service this command interracts with.
    float temperature; ///< Ambient temperature from the CLI options.

private slots:
    void temperatureCalibrated();

    friend class TestCalibrateCommand;
};
