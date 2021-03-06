// SPDX-FileCopyrightText: 2022 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "dsocommand.h"

#include <qtpokit/pokitdevice.h>

#include <QJsonDocument>
#include <QJsonObject>

/*!
 * \class DsoCommand
 *
 * The DsoCommand class implements the `dso` CLI command.
 */

/*!
 * Construct a new DsoCommand object with \a parent.
 */
DsoCommand::DsoCommand(QObject * const parent) : DeviceCommand(parent),
    service(nullptr), settings{
        DsoService::Command::FreeRunning, 0, DsoService::Mode::DcVoltage,
        { DsoService::VoltageRange::_30V_to_60V }, 1000*1000, 1000}
{

}

QStringList DsoCommand::requiredOptions(const QCommandLineParser &parser) const
{
    return DeviceCommand::requiredOptions(parser) + QStringList{
        QLatin1String("mode"),
        QLatin1String("range"),
    };
}

QStringList DsoCommand::supportedOptions(const QCommandLineParser &parser) const
{
    return DeviceCommand::supportedOptions(parser) + QStringList{
        QLatin1String("interval"),
        QLatin1String("samples"),
        QLatin1String("trigger-level"),
        QLatin1String("trigger-mode"),
    };
}

/*!
 * \copybrief DeviceCommand::processOptions
 *
 * This implementation extends DeviceCommand::processOptions to process additional CLI options
 * supported (or required) by this command.
 */
QStringList DsoCommand::processOptions(const QCommandLineParser &parser)
{
    QStringList errors = DeviceCommand::processOptions(parser);
    if (!errors.isEmpty()) {
        return errors;
    }

    // Parse the (required) mode option.
    const QString mode = parser.value(QLatin1String("mode")).trimmed().toLower();
    if (mode.startsWith(QLatin1String("ac v")) || mode.startsWith(QLatin1String("vac"))) {
        settings.mode = DsoService::Mode::AcVoltage;
    } else if (mode.startsWith(QLatin1String("dc v")) || mode.startsWith(QLatin1String("vdc"))) {
        settings.mode = DsoService::Mode::DcVoltage;
    } else if (mode.startsWith(QLatin1String("ac c")) || mode.startsWith(QLatin1String("aac"))) {
        settings.mode = DsoService::Mode::AcCurrent;
    } else if (mode.startsWith(QLatin1String("dc c")) || mode.startsWith(QLatin1String("aac"))) {
        settings.mode = DsoService::Mode::DcCurrent;
    } else {
        errors.append(tr("Unknown DSO mode: %1").arg(parser.value(QLatin1String("mode"))));
        return errors;
    }

    // Parse the (required) range option.
    QString unit;
    {
        const QString value = parser.value(QLatin1String("range"));
        quint32 sensibleMinimum = 0;
        switch (settings.mode) {
        case DsoService::Mode::Idle:
            Q_ASSERT(false); // Not possible, since the mode parsing above never allows Idle.
            break;
        case DsoService::Mode::DcVoltage:
        case DsoService::Mode::AcVoltage:
            unit = QLatin1String("V");
            sensibleMinimum = 50; // mV.
            break;
        case DsoService::Mode::DcCurrent:
        case DsoService::Mode::AcCurrent:
            unit = QLatin1String("A");
            sensibleMinimum = 5; // mA.
            break;
        }
        Q_ASSERT(!unit.isEmpty());
        const quint32 rangeMax = parseMilliValue(value, unit, sensibleMinimum);
        if (rangeMax == 0) {
            errors.append(tr("Invalid range value: %1").arg(value));
        } else {
            settings.range = lowestRange(settings.mode, rangeMax);
        }
    }

    // Parse the trigger-level option.
    if (parser.isSet(QLatin1String("trigger-level"))) {
        const QString value = parser.value(QLatin1String("trigger-level"));
        const quint32 interval = parseMilliValue(value, unit);
        if (interval == 0) {
            errors.append(tr("Invalid interval value: %1").arg(value));
        } else {
            settings.triggerLevel = interval;
        }
    }

    // Parse the trigger-mode option.
    if (parser.isSet(QLatin1String("trigger-mode"))) {
        const QString triggerMode = parser.value(QLatin1String("trigger-mode")).trimmed().toLower();
        if (triggerMode.startsWith(QLatin1String("free"))) {
            settings.command = DsoService::Command::FreeRunning;
        } else if (triggerMode.startsWith(QLatin1String("ris"))) {
           settings.command = DsoService::Command::RisingEdgeTrigger;
        } else if (triggerMode.startsWith(QLatin1String("fall"))) {
            settings.command = DsoService::Command::FallingEdgeTrigger;
        } else {
            errors.append(tr("Unknown trigger mode: %1").arg(
                parser.value(QLatin1String("trigger-mode"))));
        }
    }

    // Ensure that if either trigger option is present, then both are.
    if (parser.isSet(QLatin1String("trigger-level")) !=
        parser.isSet(QLatin1String("trigger-mode"))) {
        errors.append(tr("If either option is provided, then both must be: trigger-level, trigger-mode"));
    }

    // Parse the interval option.
    if (parser.isSet(QLatin1String("interval"))) {
        const QString value = parser.value(QLatin1String("interval"));
        const quint32 interval = parseMicroValue(value, QLatin1String("s"), 500*1000);
        if (interval == 0) {
            errors.append(tr("Invalid interval value: %1").arg(value));
        } else {
            settings.samplingWindow = interval;
        }
    }

    // Parse the samples option.
    if (parser.isSet(QLatin1String("samples"))) {
        const QString value = parser.value(QLatin1String("samples"));
        const quint32 samples = parseWholeValue(value, QLatin1String("S"));
        if (samples == 0) {
            errors.append(tr("Invalid samples value: %1").arg(value));
        } else {
            settings.numberOfSamples = samples;
        }
    }
    return errors;
}

/*!
 * \copybrief DeviceCommand::getService
 *
 * This override returns a pointer to a DsoService object.
 */
AbstractPokitService * DsoCommand::getService()
{
    Q_ASSERT(device);
    if (!service) {
        service = device->dso();
        Q_ASSERT(service);
        connect(service, &DsoService::settingsWritten,
                this, &DsoCommand::settingsWritten);
    }
    return service;
}

/*!
 * \copybrief DeviceCommand::serviceDetailsDiscovered
 *
 * This override fetches the current device's status, and outputs it in the selected format.
 */
void DsoCommand::serviceDetailsDiscovered()
{
    DeviceCommand::serviceDetailsDiscovered(); // Just logs consistently.
    const QString range = DsoService::toString(settings.range, settings.mode);
    qCInfo(lc).noquote() << tr("Sampling %1, with range %2, %L3 samples over %L4us").arg(
        DsoService::toString(settings.mode), (range.isNull()) ? QString::fromLatin1("N/A") : range)
        .arg(settings.numberOfSamples).arg(settings.samplingWindow);
    service->setSettings(settings);
}

/*!
 * Returns the lowest \a mode range that can measure at least up to \a desired max, or AutoRange
 * if no such range is available.
 */
DsoService::Range DsoCommand::lowestRange(
    const DsoService::Mode mode, const quint32 desiredMax)
{
    DsoService::Range range{ DsoService::VoltageRange::_6V_to_12V };
    switch (mode) {
    case DsoService::Mode::Idle:
        qCWarning(lc).noquote() << tr("Idle has no defined ranges.");
        Q_ASSERT(false); // Should never have been called with this Idle mode.
        break;
    case DsoService::Mode::DcVoltage:
    case DsoService::Mode::AcVoltage:
        range.voltageRange = lowestVoltageRange(desiredMax);
        break;
    case DsoService::Mode::DcCurrent:
    case DsoService::Mode::AcCurrent:
        range.currentRange = lowestCurrentRange(desiredMax);
        break;
    default:
        qCWarning(lc).noquote() << tr("No defined ranges for mode %1.").arg((quint8)mode);
        Q_ASSERT(false); // Should never have been called with this invalid mode.
    }
    return range;
}

#define POKIT_APP_IF_LESS_THAN_RETURN(value, label) \
if (value <=  DsoService::maxValue(DsoService::label).toUInt()) { \
    return DsoService::label; \
}

/*!
 * Returns the lowest current range that can measure at least up to \a desired max, or AutoRange
 * if no such range is available.
 */
DsoService::CurrentRange DsoCommand::lowestCurrentRange(const quint32 desiredMax)
{
    POKIT_APP_IF_LESS_THAN_RETURN(desiredMax, CurrentRange::_0_to_10mA)
    POKIT_APP_IF_LESS_THAN_RETURN(desiredMax, CurrentRange::_10mA_to_30mA)
    POKIT_APP_IF_LESS_THAN_RETURN(desiredMax, CurrentRange::_30mA_to_150mA)
    POKIT_APP_IF_LESS_THAN_RETURN(desiredMax, CurrentRange::_150mA_to_300mA)
    POKIT_APP_IF_LESS_THAN_RETURN(desiredMax, CurrentRange::_300mA_to_3A)
    return DsoService::CurrentRange::_300mA_to_3A; // Out of range, so go with the biggest.
}

/*!
 * Returns the lowest voltage range that can measure at least up to \a desired max, or AutoRange
 * if no such range is available.
 */
DsoService::VoltageRange DsoCommand::lowestVoltageRange(const quint32 desiredMax)
{
    POKIT_APP_IF_LESS_THAN_RETURN(desiredMax, VoltageRange::_0_to_300mV)
    POKIT_APP_IF_LESS_THAN_RETURN(desiredMax, VoltageRange::_300mV_to_2V)
    POKIT_APP_IF_LESS_THAN_RETURN(desiredMax, VoltageRange::_2V_to_6V)
    POKIT_APP_IF_LESS_THAN_RETURN(desiredMax, VoltageRange::_6V_to_12V)
    POKIT_APP_IF_LESS_THAN_RETURN(desiredMax, VoltageRange::_12V_to_30V)
    POKIT_APP_IF_LESS_THAN_RETURN(desiredMax, VoltageRange::_30V_to_60V)
    return DsoService::VoltageRange::_30V_to_60V; // Out of range, so go with the biggest.
}

#undef POKIT_APP_IF_LESS_THAN_RETURN

/*!
 * Invoked when the DSO settings have been written.
 */
void DsoCommand::settingsWritten()
{
    qCDebug(lc).noquote() << tr("Settings written; DSO has started.");
    connect(service, &DsoService::metadataRead, this, &DsoCommand::metadataRead);
    connect(service, &DsoService::samplesRead, this, &DsoCommand::outputSamples);
    service->enableMetadataNotifications();
    service->enableReadingNotifications();
}

/*!
 * Invoked when \a metadata has been received from the DSO.
 */
void DsoCommand::metadataRead(const DsoService::Metadata &metadata)
{
    qCDebug(lc) << "status:" << (int)(metadata.status);
    qCDebug(lc) << "scale:" << metadata.scale;
    qCDebug(lc) << "mode:" << DsoService::toString(metadata.mode);
    qCDebug(lc) << "range:" << DsoService::toString(metadata.range.voltageRange);
    qCDebug(lc) << "samplingWindow:" << (int)metadata.samplingWindow;
    qCDebug(lc) << "numberOfSamples:" << metadata.numberOfSamples;
    qCDebug(lc) << "samplingRate:" << metadata.samplingRate << "Hz";
    this->metadata = metadata;
    this->samplesToGo = metadata.numberOfSamples;
}

/*!
 * Outputs DSO \a samples in the selected ouput format.
 */
void DsoCommand::outputSamples(const DsoService::Samples &samples)
{
    QString unit;
    switch (metadata.mode) {
    case DsoService::Mode::DcVoltage: unit = QLatin1String("Vdc"); break;
    case DsoService::Mode::AcVoltage: unit = QLatin1String("Vac"); break;
    case DsoService::Mode::DcCurrent: unit = QLatin1String("Adc"); break;
    case DsoService::Mode::AcCurrent: unit = QLatin1String("Aac"); break;
    default:
        qCDebug(lc).noquote() << tr("No known unit for mode %1 \"%2\".").arg((int)metadata.mode)
            .arg(DsoService::toString(metadata.mode));
    }
    const QString range = DsoService::toString(metadata.range, metadata.mode);

    for (const qint16 &sample: samples) {
        static int sampleNumber = 0; ++sampleNumber;
        const float value = sample * metadata.scale;
        switch (format) {
        case OutputFormat::Csv:
            for (static bool firstTime = true; firstTime; firstTime = false) {
                fputs(qPrintable(tr("sample_number,value,unit,range\n")), stdout);
            }
            fputs(qPrintable(QString::fromLatin1("%1,%2,%3,%4\n").arg(sampleNumber).arg(value)
                .arg(unit, range)), stdout);
            break;
        case OutputFormat::Json:
            fputs(QJsonDocument(QJsonObject{
                    { QLatin1String("value"),  value },
                    { QLatin1String("unit"),   unit },
                    { QLatin1String("range"),  range },
                    { QLatin1String("mode"),   DsoService::toString(metadata.mode) },
                }).toJson(), stdout);
            break;
        case OutputFormat::Text:
            fputs(qPrintable(tr("%1 %2 %3\n").arg(sampleNumber).arg(value).arg(unit)), stdout);
            break;
        }
        --samplesToGo;
    }
    if (samplesToGo <= 0) {
        qCInfo(lc).noquote() << tr("Finished fetching %L1 samples (with %L3 to remaining).")
            .arg(metadata.numberOfSamples).arg(samplesToGo);
        disconnect(); // Will exit the application once disconnected.
    }
}
