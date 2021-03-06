// SPDX-FileCopyrightText: 2022 Paul Colby <git@colby.id.au>
// SPDX-License-Identifier: LGPL-3.0-or-later

#include "testpokitdevice.h"

#include <qtpokit/pokitdevice.h>
#include "pokitdevice_p.h"

#include <qtpokit/calibrationservice.h>
#include <qtpokit/dataloggerservice.h>
#include <qtpokit/deviceinfoservice.h>
#include <qtpokit/dsoservice.h>
#include <qtpokit/genericaccessservice.h>
#include <qtpokit/multimeterservice.h>
#include <qtpokit/statusservice.h>

void TestPokitDevice::controller()
{
    PokitDevice device(nullptr);
    QCOMPARE(device.controller(), nullptr);
}

void TestPokitDevice::controller_const()
{
    const PokitDevice device(nullptr);
    QCOMPARE(device.controller(), nullptr);
}

void TestPokitDevice::calibration()
{
    PokitDevice device(nullptr);
    const CalibrationService * service = device.calibration();
    QVERIFY(service != nullptr);
    QCOMPARE(device.calibration(), service); // Once created, the PokitDevice instance will always
    QCOMPARE(device.calibration(), service); // return the same service class instances (in a thread
    QCOMPARE(device.calibration(), service); // safe manner, too).
}

void TestPokitDevice::dataLogger()
{
    PokitDevice device(nullptr);
    const DataLoggerService * service = device.dataLogger();
    QVERIFY(service != nullptr);
    QCOMPARE(device.dataLogger(), service); // Once created, the PokitDevice instance will always
    QCOMPARE(device.dataLogger(), service); // return the same service class instances (in a thread
    QCOMPARE(device.dataLogger(), service); // safe manner, too).
}

void TestPokitDevice::deviceInformation()
{
    PokitDevice device(nullptr);
    const DeviceInfoService * service = device.deviceInformation();
    QVERIFY(service != nullptr);
    QCOMPARE(device.deviceInformation(), service); // Once created, the PokitDevice instance will always
    QCOMPARE(device.deviceInformation(), service); // return the same service class instances (in a thread
    QCOMPARE(device.deviceInformation(), service); // safe manner, too).
}

void TestPokitDevice::dso()
{
    PokitDevice device(nullptr);
    const DsoService * service = device.dso();
    QVERIFY(service != nullptr);
    QCOMPARE(device.dso(), service); // Once created, the PokitDevice instance will always
    QCOMPARE(device.dso(), service); // return the same service class instances (in a thread
    QCOMPARE(device.dso(), service); // safe manner, too).
}

void TestPokitDevice::genericAccess()
{
    PokitDevice device(nullptr);
    const GenericAccessService * service = device.genericAccess();
    QVERIFY(service != nullptr);
    QCOMPARE(device.genericAccess(), service); // Once created, the PokitDevice instance will always
    QCOMPARE(device.genericAccess(), service); // return the same service class instances (in a thread
    QCOMPARE(device.genericAccess(), service); // safe manner, too).
}

void TestPokitDevice::multimeter()
{
    PokitDevice device(nullptr);
    const MultimeterService * service = device.multimeter();
    QVERIFY(service != nullptr);
    QCOMPARE(device.multimeter(), service); // Once created, the PokitDevice instance will always
    QCOMPARE(device.multimeter(), service); // return the same service class instances (in a thread
    QCOMPARE(device.multimeter(), service); // safe manner, too).
}

void TestPokitDevice::status()
{
    PokitDevice device(nullptr);
    const StatusService * service = device.status();
    QVERIFY(service != nullptr);
    QCOMPARE(device.status(), service); // Once created, the PokitDevice instance will always
    QCOMPARE(device.status(), service); // return the same service class instances (in a thread
    QCOMPARE(device.status(), service); // safe manner, too).
}

void TestPokitDevice::serviceToString_data()
{
    QTest::addColumn<QBluetoothUuid>("uuid");
    QTest::addColumn<QString>("expected");

    #define QTPOKIT_ADD_TEST_ROW(service, expected) \
        QTest::addRow(#service) << service::serviceUuid << QStringLiteral(expected)
    QTPOKIT_ADD_TEST_ROW(CalibrationService, "Calibration");
    QTPOKIT_ADD_TEST_ROW(DataLoggerService,  "Data Logger");
    QTPOKIT_ADD_TEST_ROW(DsoService,         "DSO");
    QTPOKIT_ADD_TEST_ROW(MultimeterService,  "Multimeter");
    #undef QTPOKIT_ADD_TEST_ROW

    QTest::addRow("Status (Pokit Meter)")
        << StatusService::ServiceUuids::pokitMeter << "Status (Pokit Meter)";
    QTest::addRow("Status (Pokit Pro)")
        << StatusService::ServiceUuids::pokitPro << "Status (Pokit Pro)";

    QTest::addRow("DeviceInfoService") << DeviceInfoService::serviceUuid <<
        QBluetoothUuid::serviceClassToString(QBluetoothUuid::ServiceClassUuid::DeviceInformation);
    QTest::addRow("GenericAccessService") << GenericAccessService::serviceUuid <<
        QBluetoothUuid::serviceClassToString(QBluetoothUuid::ServiceClassUuid::GenericAccess);
}

void TestPokitDevice::serviceToString()
{
    QFETCH(QBluetoothUuid, uuid);
    QFETCH(QString, expected);
    QCOMPARE(PokitDevice::serviceToString(uuid), expected);
}

void TestPokitDevice::charcteristicToString_data()
{
    QTest::addColumn<QBluetoothUuid>("uuid");
    QTest::addColumn<QString>("expected");

    #define QTPOKIT_ADD_TEST_ROW(service, characteristic, expected) \
        QTest::addRow(#service "::" #characteristic) \
            << service::CharacteristicUuids::characteristic << QStringLiteral(expected)
    QTPOKIT_ADD_TEST_ROW(CalibrationService, temperature,           "Temperature");
    QTPOKIT_ADD_TEST_ROW(DataLoggerService,  metadata,              "Metadata");
    QTPOKIT_ADD_TEST_ROW(DataLoggerService,  reading,               "Reading");
    QTPOKIT_ADD_TEST_ROW(DataLoggerService,  settings,              "Settings");
    QTPOKIT_ADD_TEST_ROW(DsoService,         metadata,              "Metadata");
    QTPOKIT_ADD_TEST_ROW(DsoService,         reading,               "Reading");
    QTPOKIT_ADD_TEST_ROW(DsoService,         settings,              "Settings");
    QTPOKIT_ADD_TEST_ROW(MultimeterService,  reading,               "Reading");
    QTPOKIT_ADD_TEST_ROW(MultimeterService,  settings,              "Settings");
    QTPOKIT_ADD_TEST_ROW(StatusService,      deviceCharacteristics, "Device Characteristics");
    QTPOKIT_ADD_TEST_ROW(StatusService,      flashLed,              "Flash LED");
    QTPOKIT_ADD_TEST_ROW(StatusService,      name,                  "Name");
    QTPOKIT_ADD_TEST_ROW(StatusService,      status,                "Status");
    #undef QTPOKIT_ADD_TEST_ROW

    #define QTPOKIT_ADD_TEST_ROW(service, characteristic, expected) \
        QTest::addRow(#service "::" #characteristic) \
            << service::CharacteristicUuids::characteristic \
            << QBluetoothUuid::characteristicToString(QBluetoothUuid::CharacteristicType::expected)
    QTPOKIT_ADD_TEST_ROW(DeviceInfoService,    firmwareRevision, FirmwareRevisionString);
    QTPOKIT_ADD_TEST_ROW(DeviceInfoService,    hardwareRevision, HardwareRevisionString);
    QTPOKIT_ADD_TEST_ROW(DeviceInfoService,    manufacturerName, ManufacturerNameString);
    QTPOKIT_ADD_TEST_ROW(DeviceInfoService,    modelNumber,      ModelNumberString);
    QTPOKIT_ADD_TEST_ROW(DeviceInfoService,    softwareRevision, SoftwareRevisionString);
    QTPOKIT_ADD_TEST_ROW(GenericAccessService, appearance,       Appearance);
    QTPOKIT_ADD_TEST_ROW(GenericAccessService, deviceName,       DeviceName);
    #undef QTPOKIT_ADD_TEST_ROW
}

void TestPokitDevice::charcteristicToString()
{
    QFETCH(QBluetoothUuid, uuid);
    QFETCH(QString, expected);
    QCOMPARE(PokitDevice::charcteristicToString(uuid), expected);
}

void TestPokitDevice::setController()
{
    // Verify safe error handling (can't do much else without a Bluetooth device).
    PokitDevice device(nullptr);
    QCOMPARE(device.controller(), nullptr);

    QLowEnergyController * const controller1 =
        QLowEnergyController::createCentral(QBluetoothDeviceInfo());
    device.d_func()->setController(controller1);
    QCOMPARE(device.controller(), controller1);

    QLowEnergyController * const controller2 =
        QLowEnergyController::createCentral(QBluetoothDeviceInfo());
    device.d_func()->setController(controller2);
    QCOMPARE(device.controller(), controller2);

    // Safe to set the same controller.
    device.d_func()->setController(controller2);
    QCOMPARE(device.controller(), controller2);
    device.d_func()->setController(controller2);
    QCOMPARE(device.controller(), controller2);

    // Safe to "unset" the controller.
    device.d_func()->setController(nullptr);
    QCOMPARE(device.controller(), nullptr);

    // Safe to clean-up now.
    delete controller1;
    delete controller2;
}

void TestPokitDevice::connected()
{
    // Verify safe error handling (can't do much else without a Bluetooth device).
    PokitDevice device(nullptr);
    QTest::ignoreMessage(QtCriticalMsg,
        "PokitDevicePrivate::connected slot invoked without a controller.");
    device.d_func()->connected();

    QLowEnergyController * const controller =
        QLowEnergyController::createCentral(QBluetoothDeviceInfo());
    device.d_func()->setController(controller);
    device.d_func()->connected();

    device.d_func()->setController(nullptr);
    QTest::ignoreMessage(QtCriticalMsg,
        "PokitDevicePrivate::connected slot invoked without a controller.");
    device.d_func()->connected();

    delete controller;
}

void TestPokitDevice::connectionUpdated()
{
    // Verify safe error handling (can't do much else without a Bluetooth device).
    PokitDevice device(nullptr);
    device.d_func()->connectionUpdated(QLowEnergyConnectionParameters());
}

void TestPokitDevice::disconnected()
{
    // Verify safe error handling (can't do much else without a Bluetooth device).
    PokitDevice device(nullptr);
    device.d_func()->disconnected();
}

void TestPokitDevice::discoveryFinished()
{
    // Verify safe error handling (can't do much else without a Bluetooth device).
    PokitDevice device(nullptr);
    device.d_func()->discoveryFinished();
}

void TestPokitDevice::errorOccurred()
{
    // Verify safe error handling (can't do much else without a Bluetooth device).
    PokitDevice device(nullptr);
    device.d_func()->errorOccurred(QLowEnergyController::Error::UnknownError);
}

void TestPokitDevice::serviceDiscovered()
{
    // Verify safe error handling (can't do much else without a Bluetooth device).
    PokitDevice device(nullptr);
    device.d_func()->serviceDiscovered(QBluetoothUuid::createUuid());
}

void TestPokitDevice::stateChanged()
{
    // Verify safe error handling (can't do much else without a Bluetooth device).
    PokitDevice device(nullptr);
    device.d_func()->stateChanged(QLowEnergyController::ControllerState::ClosingState);
}

QTEST_MAIN(TestPokitDevice)
