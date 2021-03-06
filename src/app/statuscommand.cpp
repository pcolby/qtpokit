// SPDX-FileCopyrightText: 2022 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "statuscommand.h"

#include <qtpokit/pokitdevice.h>
#include <qtpokit/statusservice.h>

#include <QJsonDocument>
#include <QJsonObject>

/*!
 * \class StatusCommand
 *
 * The StatusCommand class implements the `status` CLI command.
 */

/*!
 * Construct a new StatusCommand object with \a parent.
 */
StatusCommand::StatusCommand(QObject * const parent) : DeviceCommand(parent), service(nullptr)
{

}

QStringList StatusCommand::requiredOptions(const QCommandLineParser &parser) const
{
    return DeviceCommand::requiredOptions(parser) + QStringList{
    };
}

QStringList StatusCommand::supportedOptions(const QCommandLineParser &parser) const
{
    return DeviceCommand::supportedOptions(parser);
}

/*!
 * \copybrief DeviceCommand::processOptions
 *
 * This implementation extends DeviceCommand::processOptions to process additional CLI options
 * supported (or required) by this command.
 */
QStringList StatusCommand::processOptions(const QCommandLineParser &parser)
{
    QStringList errors = DeviceCommand::processOptions(parser);
    if (!errors.isEmpty()) {
        return errors;
    }

    return errors;
}

/*!
 * \copybrief DeviceCommand::getService
 *
 * This override returns a pointer to a StatusService object.
 */
AbstractPokitService * StatusCommand::getService()
{
    Q_ASSERT(device);
    if (!service) {
        service = device->status();
        Q_ASSERT(service);
    }
    return service;
}

/*!
 * \copybrief DeviceCommand::serviceDetailsDiscovered
 *
 * This override fetches the current device's status, and outputs it in the selected format.
 */
void StatusCommand::serviceDetailsDiscovered()
{
    DeviceCommand::serviceDetailsDiscovered(); // Just logs consistently.
    const QString deviceName = service->deviceName();
    const StatusService::Status status = service->status();
    const QString statusLabel = StatusService::toString(status.deviceStatus);
    const QString batteryLabel = StatusService::toString(status.batteryStatus);
    const StatusService::DeviceCharacteristics chrs = service->deviceCharacteristics();
    if (chrs.firmwareVersion.isNull()) {
        qCWarning(lc).noquote() << tr("Failed to parse device information");
        QCoreApplication::exit(EXIT_FAILURE);
        return;
    }

    switch (format) {
    case OutputFormat::Csv:
        fputs(qPrintable(tr("device_name,device_status,firmware_version,maximum_voltage,"
                            "maximum_current,maximum_resistance,maximum_sampling_rate,"
                            "sampling_buffer_size,capability_mask,mac_address,battery_voltage,"
                            "battery_status\n")), stdout);
        fputs(qPrintable(QString::fromLatin1("%1,%2,%3,%4,%5,%6,%7,%8,%9,%10,%11,%12\n")
            .arg(escapeCsvField(deviceName),statusLabel.toLower(),chrs.firmwareVersion.toString())
            .arg(chrs.maximumVoltage).arg(chrs.maximumCurrent).arg(chrs.maximumResistance)
            .arg(chrs.maximumSamplingRate).arg(chrs.samplingBufferSize).arg(chrs.capabilityMask)
            .arg(chrs.macAddress.toString()).arg(status.batteryVoltage)
            .arg(batteryLabel.toLower())), stdout);
        break;
    case OutputFormat::Json: {
        QJsonObject battery{
            { QLatin1String("level"),  status.batteryVoltage },
        };
        if (!batteryLabel.isNull()) {
            battery.insert(QLatin1String("status"), batteryLabel);
        }
        fputs(QJsonDocument(QJsonObject{
                { QLatin1String("deviceName"),   deviceName },
                { QLatin1String("firmwareVersion"), QJsonObject{
                      { QLatin1String("major"), chrs.firmwareVersion.majorVersion() },
                      { QLatin1String("minor"), chrs.firmwareVersion.minorVersion() },
                }},
                { QLatin1String("maximumVoltage"),      chrs.maximumVoltage },
                { QLatin1String("maximumCurrent"),      chrs.maximumCurrent },
                { QLatin1String("maximumResistance"),   chrs.maximumResistance },
                { QLatin1String("maximumSamplingRate"), chrs.maximumSamplingRate },
                { QLatin1String("samplingBufferSize"),  chrs.samplingBufferSize },
                { QLatin1String("capabilityMask"),      chrs.capabilityMask },
                { QLatin1String("macAddress"),          chrs.macAddress.toString() },
                { QLatin1String("deviceStatus"), QJsonObject{
                      { QLatin1String("code"), (quint8)status.deviceStatus },
                      { QLatin1String("label"), statusLabel },
                }},
                { QLatin1String("battery"), battery },
            }).toJson(), stdout);
    }   break;
    case OutputFormat::Text:
        fputs(qPrintable(tr("Device name:           %1\n").arg(deviceName)), stdout);
        fputs(qPrintable(tr("Firmware version:      %1\n").arg(chrs.firmwareVersion.toString())), stdout);
        fputs(qPrintable(tr("Maximum voltage:       %1\n").arg(chrs.maximumVoltage)), stdout);
        fputs(qPrintable(tr("Maximum current:       %1\n").arg(chrs.maximumCurrent)), stdout);
        fputs(qPrintable(tr("Maximum resistance:    %1\n").arg(chrs.maximumResistance)), stdout);
        fputs(qPrintable(tr("Maximum sampling rate: %1\n").arg(chrs.maximumSamplingRate)), stdout);
        fputs(qPrintable(tr("Sampling buffer size:  %1\n").arg(chrs.samplingBufferSize)), stdout);
        fputs(qPrintable(tr("Capability mask:       %1\n").arg(chrs.capabilityMask)), stdout);
        fputs(qPrintable(tr("MAC address:           %1\n").arg(chrs.macAddress.toString())), stdout);
        fputs(qPrintable(tr("Device status:         %1 (%2)\n").arg(statusLabel)
            .arg((quint8)status.deviceStatus)), stdout);
        fputs(qPrintable(tr("Battery voltage:       %1\n").arg(status.batteryVoltage)), stdout);
        fputs(qPrintable(tr("Battery status:        %1 (%2)\n")
            .arg(batteryLabel.isNull() ? QString::fromLatin1("N/A") : batteryLabel)
            .arg((quint8)status.batteryStatus)), stdout);
        break;
    }
    disconnect(); // Will exit the application once disconnected.
}
