// SPDX-FileCopyrightText: 2022 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "calibratecommand.h"
#include "dsocommand.h"
#include "flashledcommand.h"
#include "infocommand.h"
#include "loggerfetchcommand.h"
#include "loggerstartcommand.h"
#include "loggerstopcommand.h"
#include "metercommand.h"
#include "scancommand.h"
#include "setnamecommand.h"
#include "statuscommand.h"

#include <QCommandLineParser>
#include <QCoreApplication>
#include <QLoggingCategory>

#if defined(Q_OS_UNIX)
#include <unistd.h>
#elif defined(Q_OS_WIN)
#include <Windows.h>
#endif

inline bool haveConsole()
{
    #if defined(Q_OS_UNIX)
    return isatty(STDERR_FILENO);
    #elif defined(Q_OS_WIN)
    return GetConsoleWindow();
    #else
    return false;
    #endif
}

void configureLogging(const QCommandLineParser &parser)
{
    // Start with the Qt default message pattern (see qtbase:::qlogging.cpp:defaultPattern)
    QString messagePattern = QStringLiteral("%{if-category}%{category}: %{endif}%{message}");

    if (parser.isSet(QStringLiteral("debug"))) {
        #ifdef QT_MESSAGELOGCONTEXT
        // %{file}, %{line} and %{function} are only available when QT_MESSAGELOGCONTEXT is set.
        messagePattern.prepend(QStringLiteral("%{function} "));
        #endif
        messagePattern.prepend(QStringLiteral("%{time process} %{type} "));
        QLoggingCategory::setFilterRules(QStringLiteral("pokit.*.debug=true"));
    }

    const QString color = parser.value(QStringLiteral("color"));
    if ((color == QStringLiteral("yes")) || (color == QStringLiteral("auto") && haveConsole())) {
        messagePattern.prepend(QStringLiteral(
        "%{if-debug}\x1b[37m%{endif}"      // White
        "%{if-info}\x1b[32m%{endif}"       // Green
        "%{if-warning}\x1b[35m%{endif}"    // Magenta
        "%{if-critical}\x1b[31m%{endif}"   // Red
        "%{if-fatal}\x1b[31;1m%{endif}")); // Red and bold
        messagePattern.append(QStringLiteral("\x1b[0m")); // Reset.
    }

    qSetMessagePattern(messagePattern);
}

enum class Command {
    None,
    Info,
    Status,
    Meter,
    DSO,
    LoggerStart,
    LoggerStop,
    LoggerFetch,
    Scan,
    SetName,
    FlashLed,
    Calibrate
};

void showCliError(const QString &errorText) {
    // Output the same way QCommandLineParser does (qcommandlineparser.cpp::showParserMessage).
    const QString message = QCoreApplication::applicationName() + QLatin1String(": ")
        + errorText + QLatin1Char('\n');
    fputs(qPrintable(message), stderr);
}

Command getCliCommand(const QStringList &posArguments) {
    if (posArguments.isEmpty()) {
        return Command::None;
    }
    if (posArguments.size() > 1) {
        showCliError(QObject::tr("More than one command: %1").arg(posArguments.join(QStringLiteral(", "))));
        ::exit(EXIT_FAILURE);
    }

    const QMap<QString, Command> supportedCommands {
        { QStringLiteral("info"),         Command::Info },
        { QStringLiteral("status"),       Command::Status },
        { QStringLiteral("meter"),        Command::Meter },
        { QStringLiteral("dso"),          Command::DSO },
        { QStringLiteral("logger-start"), Command::LoggerStart },
        { QStringLiteral("logger-stop"),  Command::LoggerStop },
        { QStringLiteral("logger-fetch"), Command::LoggerFetch },
        { QStringLiteral("scan"),         Command::Scan },
        { QStringLiteral("set-name"),     Command::SetName },
        { QStringLiteral("flash-led"),    Command::FlashLed },
        { QStringLiteral("calibrate"),    Command::Calibrate },
    };
    const Command command = supportedCommands.value(posArguments.first().toLower(), Command::None);
    if (command == Command::None) {
        showCliError(QObject::tr("Unknown command: %1").arg(posArguments.first()));
        ::exit(EXIT_FAILURE);
    }
    return command;
}

Command parseCommandLine(const QStringList &appArguments, QCommandLineParser &parser)
{
    // Setupt the command line options.
    parser.addOptions({
        { QStringLiteral("color"),
          QCoreApplication::translate("parseCommandLine", "Colors the console output. Valid options "
          "are: yes, no and auto. The default is auto."),
          QStringLiteral("yes|no|auto"), QStringLiteral("auto")},
        {{QStringLiteral("debug")},
          QCoreApplication::translate("parseCommandLine", "Enable debug output.")},
        {{QStringLiteral("d"), QStringLiteral("device")},
          QCoreApplication::translate("parseCommandLine",
          "Set the name, hardware address or MacOS UUID of Pokit device to use. If not specified, "
          "the first discovered Pokit device will be used."),
          QCoreApplication::translate("parseCommandLine", "device")},
    });
    parser.addHelpOption();
    parser.addOptions({
        {{QStringLiteral("interval")},
          QCoreApplication::translate("parseCommandLine", "Set the update interval for DOS, meter and "
          "logger modes. Suffixes such as 's' and 'ms' (for seconds and milliseconds) may be used. "
          "If no suffix is present, the units will be inferred from the magnitide of the given "
          "interval. If the option itself is not specified, a sensible default will be chosen "
          "according to the selected command."),
          QCoreApplication::translate("parseCommandLine", "interval")},
        {{QStringLiteral("mode")},
          QCoreApplication::translate("parseCommandLine", "Set the desired operation mode for "
          "meter, dso and logger modes. Supported modes are: AC Voltage, DC Voltage, AC Current, "
          "DC Current, Resistance, Diode, Continuity, and Temperature. All are case insensitive. "
          "Only the first four options are available for dso and logger commands; the rest are "
          "available in meter mode only." ),
          QCoreApplication::translate("parseCommandLine", "mode")},
        {{QStringLiteral("new-name")},
          QCoreApplication::translate("parseCommandLine","Give the desired new name for the set-"
          "name command."), QCoreApplication::translate("parseCommandLine", "name")},
        {{QStringLiteral("output")},
          QCoreApplication::translate("parseCommandLine","Set the format for output. Supported "
          "formats are: CSV, JSON and Text. All are case insenstitve. The default is Text."),
          QCoreApplication::translate("parseCommandLine", "format"),
          QCoreApplication::translate("parseCommandLine", "text")},
        {{QStringLiteral("range")},
          QCoreApplication::translate("parseCommandLine","Set the desired measurement range. Pokit "
          "devices support specific ranges, such as 0 to 300mV. Specify the desired upper limit, "
          "and the best range will be selected, or use 'auto' to enable the Pokit device's auto-"
          "range feature. The default is 'auto'."),
          QCoreApplication::translate("parseCommandLine", "range"), QStringLiteral("auto")},
        {{QStringLiteral("samples")},
          QCoreApplication::translate("parseCommandLine","Set the number of samples to acquire."),
          QCoreApplication::translate("parseCommandLine", "count")},
        {{QStringLiteral("temperature")},
          QCoreApplication::translate("parseCommandLine","Set the current ambient temperature for "
          "the calibration command."), QCoreApplication::translate("parseCommandLine", "degrees")},
        {{QStringLiteral("timeout")},
          QCoreApplication::translate("parseCommandLine","Set the device discovery scan timeout."
          "Suffixes such as 's' and 'ms' (for seconds and milliseconds) may be used. "
          "If no suffix is present, the units will be inferred from the magnitide of the given "
          "interval. The default behaviour is no timeout."),
          QCoreApplication::translate("parseCommandLine","period")},
        {{QStringLiteral("timestamp")},
          QCoreApplication::translate("parseCommandLine","Set the optional starting timestamp for "
          "data logging. Default to 'now'."),
        QCoreApplication::translate("parseCommandLine","period")},
        {{QStringLiteral("trigger-level")},
          QCoreApplication::translate("parseCommandLine","Set the DSO trigger level."),
          QCoreApplication::translate("parseCommandLine", "level")},
        {{QStringLiteral("trigger-mode")},
          QCoreApplication::translate("parseCommandLine","Set the DSO trigger mode. Supported "
          "modes are: free, rising and falling. The default is free."),
          QCoreApplication::translate("parseCommandLine", "mode"), QStringLiteral("free")},
    });
    parser.addVersionOption();

    // Add supported 'commands' (as positional arguments, so they'll appear in the help text).
    parser.addPositionalArgument(QStringLiteral("info"),
        QCoreApplication::translate("parseCommandLine", "Get Pokit device information"),
        QStringLiteral(" "));
    parser.addPositionalArgument(QStringLiteral("status"),
        QCoreApplication::translate("parseCommandLine", "Get Pokit device status"),
        QStringLiteral(" "));
    parser.addPositionalArgument(QStringLiteral("meter"),
        QCoreApplication::translate("parseCommandLine", "Access Pokit device's multimeter mode"),
        QStringLiteral(" "));
    parser.addPositionalArgument(QStringLiteral("dso"),
        QCoreApplication::translate("parseCommandLine", "Access Pokit device's DSO mode"),
        QStringLiteral(" "));
    parser.addPositionalArgument(QStringLiteral("logger-start"),
        QCoreApplication::translate("parseCommandLine", "Start Pokit device's data logger mode"),
        QStringLiteral(" "));
    parser.addPositionalArgument(QStringLiteral("logger-stop"),
        QCoreApplication::translate("parseCommandLine", "Stop Pokit device's data logger mode"),
        QStringLiteral(" "));
    parser.addPositionalArgument(QStringLiteral("logger-fetch"),
        QCoreApplication::translate("parseCommandLine", "Fetch Pokit device's data logger samples"),
        QStringLiteral(" "));
    parser.addPositionalArgument(QStringLiteral("scan"),
        QCoreApplication::translate("parseCommandLine", "Scan Bluetooth for Pokit devices"),
        QStringLiteral(" "));
    parser.addPositionalArgument(QStringLiteral("set-name"),
        QCoreApplication::translate("parseCommandLine", "Set Pokit device's name"),
        QStringLiteral(" "));
    parser.addPositionalArgument(QStringLiteral("flash-led"),
        QCoreApplication::translate("parseCommandLine", "Flash Pokit device's LED"),
        QStringLiteral(" "));
    parser.addPositionalArgument(QStringLiteral("calibrate"),
        QCoreApplication::translate("parseCommandLine", "Calibrate Pokit device temperature"),
        QStringLiteral(" "));

    // Do the initial parse, the see if we have a command specified yet.
    parser.parse(appArguments);
    configureLogging(parser);
    const Command command = getCliCommand(parser.positionalArguments());

    // If we have a (single, valid) command, then remove the commands list from the help text.
    if (command != Command::None) {
        parser.clearPositionalArguments();
    }

    // Handle -h|--help explicitly, so we can tweak the output to include the <command> info.
    if (parser.isSet(QStringLiteral("help"))) {
        const QString commandString = (command == Command::None) ? QStringLiteral("<command>")
                : parser.positionalArguments().constFirst();
        fputs(qPrintable(parser.helpText()
            .replace(QStringLiteral("[options]"), commandString + QStringLiteral(" [options]"))
            .replace(QStringLiteral("Arguments:"),  QStringLiteral("Command:"))
        ), stdout);
        ::exit(EXIT_SUCCESS);
    }

    // Process the command for real (ie throw errors for unknown options, etc).
    parser.process(appArguments);
    return command;
}

AbstractCommand * getCommandObject(const Command command, QObject * const parent)
{
    switch (command) {
    case Command::None:
        showCliError(QCoreApplication::translate("main",
            "Missing argument: <command>\nSee --help for usage information."));
        return nullptr;
    case Command::Calibrate:   return new CalibrateCommand(parent);
    case Command::DSO:         return new DsoCommand(parent);
    case Command::FlashLed:    return new FlashLedCommand(parent);
    case Command::Info:        return new InfoCommand(parent);
    case Command::LoggerStart: return new LoggerStartCommand(parent);
    case Command::LoggerStop:  return new LoggerStopCommand(parent);
    case Command::LoggerFetch: return new LoggerFetchCommand(parent);
    case Command::Meter:       return new MeterCommand(parent);
    case Command::Scan:        return new ScanCommand(parent);
    case Command::Status:      return new StatusCommand(parent);
    case Command::SetName:     return new SetNameCommand(parent);
    }
    showCliError(QCoreApplication::translate("main", "Unknown command (%1)").arg((int)command));
    return nullptr;
}

int main(int argc, char *argv[])
{
    // Setup the core application.
    QCoreApplication app(argc, argv);
    app.setApplicationName(QStringLiteral(CMAKE_PROJECT_NAME));
    #ifdef POKIT_PRE_RELEASE
    app.setApplicationVersion(QStringLiteral(CMAKE_PROJECT_VERSION "-" POKIT_PRE_RELEASE));
    #else
    app.setApplicationVersion(QStringLiteral(CMAKE_PROJECT_VERSION));
    #endif

    // Parse the command line.
    const QStringList appArguments = app.arguments();
    QCommandLineParser parser;
    const Command commandType = parseCommandLine(appArguments, parser);

    // Handle the given command.
    AbstractCommand * const command = getCommandObject(commandType, &app);
    if (command == nullptr) {
        return EXIT_FAILURE; // getCommandObject will have logged the reason already.
    }
    const QStringList cliErrors = command->processOptions(parser);
    for (const QString &error: cliErrors) {
        showCliError(error);
    }
    if (!cliErrors.isEmpty()) {
        return EXIT_FAILURE;
    }
    return (command->start()) ? app.exec() : EXIT_FAILURE;
}
