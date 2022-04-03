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

#include "discover.h"
#include "uuids.h"

#include <QCoreApplication>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLowEnergyController>

QJsonArray toJsonArray(const QBluetoothDeviceInfo::ServiceClasses &classes)
{
    QJsonArray array;
    #define TEST_THEN_ADD(flag) if (classes.testFlag(QBluetoothDeviceInfo::flag)) array.append(QLatin1String(#flag))
    TEST_THEN_ADD(PositioningService);
    TEST_THEN_ADD(NetworkingService);
    TEST_THEN_ADD(RenderingService);
    TEST_THEN_ADD(CapturingService);
    TEST_THEN_ADD(ObjectTransferService);
    TEST_THEN_ADD(AudioService);
    TEST_THEN_ADD(TelephonyService);
    TEST_THEN_ADD(InformationService);
    #undef TEST_THEN_ADD
    return array;
}

QJsonArray toJsonArray(const QList<QBluetoothUuid> &uuids)
{
    QJsonArray array;
    for (const QBluetoothUuid &uuid: uuids) {
        array.append(uuid.toString());
    }
    return array;
}

QJsonArray toJsonArray(const QMultiHash<quint16, QByteArray> &data)
{
    QJsonArray array;
    for (auto iter = data.cbegin(); iter != data.cend(); ++iter) {
        array.append(QJsonObject{
            { QLatin1String("manufacturerId"), iter.key() },
            { QLatin1String("data"), QString::fromLatin1(iter.value().toBase64()) },
        });
    }
    return array;
}

QString toString(const QBluetoothDeviceInfo::MajorDeviceClass &majorClass)
{
    #define TEST_THEN_RETURN(value) if (majorClass == QBluetoothDeviceInfo::value) return QLatin1String(#value)
    TEST_THEN_RETURN(MiscellaneousDevice);
    TEST_THEN_RETURN(ComputerDevice);
    TEST_THEN_RETURN(PhoneDevice);
    TEST_THEN_RETURN(NetworkDevice);
    TEST_THEN_RETURN(AudioVideoDevice);
    TEST_THEN_RETURN(PeripheralDevice);
    TEST_THEN_RETURN(ImagingDevice);
    TEST_THEN_RETURN(WearableDevice);
    TEST_THEN_RETURN(ToyDevice);
    TEST_THEN_RETURN(HealthDevice);
    TEST_THEN_RETURN(UncategorizedDevice);
    #undef TEST_THEN_RETURN
    return QString::number(majorClass);
}

QString toString(const QBluetoothDeviceInfo::MajorDeviceClass &majorClass, const quint8 minorClass)
{
    #define TEST_THEN_RETURN(value) if (minorClass == QBluetoothDeviceInfo::value) return QLatin1String(#value)
    switch (majorClass) {
    case QBluetoothDeviceInfo::MiscellaneousDevice:
        TEST_THEN_RETURN(UncategorizedMiscellaneous);
        break;
    case QBluetoothDeviceInfo::ComputerDevice:
        TEST_THEN_RETURN(UncategorizedComputer);
        TEST_THEN_RETURN(DesktopComputer);
        TEST_THEN_RETURN(ServerComputer);
        TEST_THEN_RETURN(LaptopComputer);
        TEST_THEN_RETURN(HandheldClamShellComputer);
        TEST_THEN_RETURN(HandheldComputer);
        TEST_THEN_RETURN(WearableComputer);
        break;
    case QBluetoothDeviceInfo::PhoneDevice:
        TEST_THEN_RETURN(UncategorizedPhone);
        TEST_THEN_RETURN(CellularPhone);
        TEST_THEN_RETURN(CordlessPhone);
        TEST_THEN_RETURN(SmartPhone);
        TEST_THEN_RETURN(WiredModemOrVoiceGatewayPhone);
        TEST_THEN_RETURN(CommonIsdnAccessPhone);
        break;
    case QBluetoothDeviceInfo::NetworkDevice:
        TEST_THEN_RETURN(NetworkFullService);
        TEST_THEN_RETURN(NetworkLoadFactorOne);
        TEST_THEN_RETURN(NetworkLoadFactorTwo);
        TEST_THEN_RETURN(NetworkLoadFactorThree);
        TEST_THEN_RETURN(NetworkLoadFactorFour);
        TEST_THEN_RETURN(NetworkLoadFactorFive);
        TEST_THEN_RETURN(NetworkLoadFactorSix);
        TEST_THEN_RETURN(NetworkNoService);
        break;
    case QBluetoothDeviceInfo::AudioVideoDevice:
        TEST_THEN_RETURN(UncategorizedAudioVideoDevice);
        TEST_THEN_RETURN(WearableHeadsetDevice);
        TEST_THEN_RETURN(HandsFreeDevice);
        TEST_THEN_RETURN(Microphone);
        TEST_THEN_RETURN(Loudspeaker);
        TEST_THEN_RETURN(Headphones);
        TEST_THEN_RETURN(PortableAudioDevice);
        TEST_THEN_RETURN(CarAudio);
        TEST_THEN_RETURN(SetTopBox);
        TEST_THEN_RETURN(HiFiAudioDevice);
        TEST_THEN_RETURN(Vcr);
        TEST_THEN_RETURN(VideoCamera);
        TEST_THEN_RETURN(Camcorder);
        TEST_THEN_RETURN(VideoMonitor);
        TEST_THEN_RETURN(VideoDisplayAndLoudspeaker);
        TEST_THEN_RETURN(VideoConferencing);
        TEST_THEN_RETURN(GamingDevice);
        break;
    case QBluetoothDeviceInfo::PeripheralDevice:
        TEST_THEN_RETURN(UncategorizedPeripheral);
        TEST_THEN_RETURN(KeyboardPeripheral);
        TEST_THEN_RETURN(PointingDevicePeripheral);
        TEST_THEN_RETURN(KeyboardWithPointingDevicePeripheral);
        TEST_THEN_RETURN(JoystickPeripheral);
        TEST_THEN_RETURN(GamepadPeripheral);
        TEST_THEN_RETURN(RemoteControlPeripheral);
        TEST_THEN_RETURN(SensingDevicePeripheral);
        TEST_THEN_RETURN(DigitizerTabletPeripheral);
        TEST_THEN_RETURN(CardReaderPeripheral);
        break;
    case QBluetoothDeviceInfo::ImagingDevice:
        TEST_THEN_RETURN(UncategorizedImagingDevice);
        TEST_THEN_RETURN(ImageDisplay);
        TEST_THEN_RETURN(ImageCamera);
        TEST_THEN_RETURN(ImageScanner);
        TEST_THEN_RETURN(ImagePrinter);
        break;
    case QBluetoothDeviceInfo::WearableDevice:
        TEST_THEN_RETURN(UncategorizedWearableDevice);
        TEST_THEN_RETURN(WearableWristWatch);
        TEST_THEN_RETURN(WearablePager);
        TEST_THEN_RETURN(WearableJacket);
        TEST_THEN_RETURN(WearableHelmet);
        TEST_THEN_RETURN(WearableGlasses);
        break;
    case QBluetoothDeviceInfo::ToyDevice:
        TEST_THEN_RETURN(UncategorizedToy);
        TEST_THEN_RETURN(ToyRobot);
        TEST_THEN_RETURN(ToyVehicle);
        TEST_THEN_RETURN(ToyDoll);
        TEST_THEN_RETURN(ToyController);
        TEST_THEN_RETURN(ToyGame);
        break;
    case QBluetoothDeviceInfo::HealthDevice:
        TEST_THEN_RETURN(UncategorizedHealthDevice);
        TEST_THEN_RETURN(HealthBloodPressureMonitor);
        TEST_THEN_RETURN(HealthThermometer);
        TEST_THEN_RETURN(HealthWeightScale);
        TEST_THEN_RETURN(HealthGlucoseMeter);
        TEST_THEN_RETURN(HealthPulseOximeter);
        TEST_THEN_RETURN(HealthDataDisplay);
        TEST_THEN_RETURN(HealthStepCounter);
        break;
    case QBluetoothDeviceInfo::UncategorizedDevice:
        // There are no minor classes defined (in Qt) for uncategorized devices.
        break;
    }
    #undef TEST_THEN_RETURN
    // Fallback to just converting the unsigned integer to a string.
    return QString::number(minorClass);
}

Discover::Discover(QObject * const parent) : QObject(parent)
{
    discoveryAgent = new PokitDeviceDiscoveryAgent(this);

    connect(discoveryAgent, &PokitDeviceDiscoveryAgent::pokitDeviceDiscovered, this, [this](const QBluetoothDeviceInfo &info) {
        QLowEnergyController * c = QLowEnergyController::createCentral(info);
        qDebug() << c;
        qDebug() << c->services().size();

        connect(c, &QLowEnergyController::connected, this, [c]() {
            qDebug() << "device connected";
            c->discoverServices();
        });

        QJsonObject json{
            { QLatin1String("address"), info.address().toString() },
            { QLatin1String("name"), info.name() },
            { QLatin1String("isCached"), info.isCached() },
            { QLatin1String("majorDeviceClass"), info.majorDeviceClass() },
            { QLatin1String("majorDeviceClass"), toString(info.majorDeviceClass()) },
            { QLatin1String("minorDeviceClass"), toString(info.majorDeviceClass(), info.minorDeviceClass()) },
            { QLatin1String("signalStrength"), info.rssi() },
            { QLatin1String("serviceUuids"), toJsonArray(info.serviceUuids()) },

        };
        if (!info.deviceUuid().isNull()) {
            json.insert(QLatin1String("deviceUuid"), info.deviceUuid().toString());
        }
        if (!info.manufacturerData().isEmpty()) {
            json.insert(QLatin1String("manufacturerData"), toJsonArray(info.manufacturerData()));
        }
        if (info.serviceClasses() != QBluetoothDeviceInfo::NoService) {
            json.insert(QLatin1String("serviceClasses"), toJsonArray(info.serviceClasses()));
        }
        fputs(QJsonDocument(json).toJson(), stdout);

        connect(c, &QLowEnergyController::serviceDiscovered, this, [c](const QBluetoothUuid &service) {
            qDebug() << "service discovered" << service;
            if (service == QBluetoothUuid(QStringLiteral(POKIT_SERVICE_STATUS))) {
                qDebug() << "status";
                QLowEnergyService * s = c->createServiceObject(service);
                qDebug() << s;
                if (s != nullptr) {
                    s->discoverDetails();
                }
            }
        });

        connect(c, &QLowEnergyController::discoveryFinished, this, [c]() {
            qDebug() << "service discovery finished" << c->services().size();
            QCoreApplication::quit();
        });

        c->connectToDevice();
    });

    connect(discoveryAgent, &PokitDeviceDiscoveryAgent::finished, this, []() {
        qDebug() << "device discovery finished";
//        QCoreApplication::quit();
    });

    discoveryAgent->setLowEnergyDiscoveryTimeout(3000);
    discoveryAgent->start();
}