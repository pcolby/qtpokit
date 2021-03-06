# SPDX-FileCopyrightText: 2022 Paul Colby <git@colby.id.au>
# SPDX-License-Identifier: LGPL-3.0-or-later

cmake_minimum_required(VERSION 3.0)

find_package(Qt${QT_VERSION_MAJOR}Test REQUIRED)

# Library Unit Tests

function(add_pokit_unit_test name)
  add_executable(test${name} ${ARGN})

  target_include_directories(test${name} PRIVATE ${CMAKE_SOURCE_DIR}/src/lib)

  target_link_libraries(
    test${name}
    PRIVATE QtPokit
    PRIVATE Qt${QT_VERSION_MAJOR}::Bluetooth
    PRIVATE Qt${QT_VERSION_MAJOR}::Test)

if(${QT_VERSION} VERSION_LESS "5.12")
  add_test(NAME ${name} COMMAND test${name})
else() # Support for TAP format added to Qt Test in 5.12.
  add_test(NAME ${name} COMMAND test${name} -o ${name}.tap,tap -o -,txt)
endif()
  set_tests_properties(${name} PROPERTIES LABELS "lib;unit")
endfunction()

add_pokit_unit_test(
  AbstractPokitService
  testabstractpokitservice.cpp
  testabstractpokitservice.h)

add_pokit_unit_test(
  CalibrationService
  testcalibrationservice.cpp
  testcalibrationservice.h)

add_pokit_unit_test(
  DataLoggerService
  testdataloggerservice.cpp
  testdataloggerservice.h)

add_pokit_unit_test(
  DeviceInfoService
  testdeviceinfoservice.cpp
  testdeviceinfoservice.h)

add_pokit_unit_test(
  DsoService
  testdsoservice.cpp
  testdsoservice.h)

add_pokit_unit_test(
  GenericAccessService
  testgenericaccessservice.cpp
  testgenericaccessservice.h)

add_pokit_unit_test(
  MultimeterService
  testmultimeterservice.cpp
  testmultimeterservice.h)

add_pokit_unit_test(
  PokitDevice
  testpokitdevice.cpp
  testpokitdevice.h)

add_pokit_unit_test(
  PokitDiscoveryAgent
  testpokitdiscoveryagent.cpp
  testpokitdiscoveryagent.h)

add_pokit_unit_test(
  StatusService
  teststatusservice.cpp
  teststatusservice.h)

# App Unit Tests

function(add_pokit_app_unit_test name)
if(${CMAKE_VERSION} VERSION_LESS "3.12.0")
  message("-- Skipping test${name} (needs CMake 3.12+)")
else()
  add_pokit_unit_test(${name} ${ARGN})
  set_tests_properties(${name} PROPERTIES LABELS "app;unit")
  target_include_directories(test${name} PRIVATE ${CMAKE_SOURCE_DIR}/src/app)
  target_link_libraries(test${name} PRIVATE PokitApp)
endif()
endfunction()

add_pokit_app_unit_test(
  AbstractCommand
  testabstractcommand.cpp
  testabstractcommand.h)

add_pokit_app_unit_test(
  CalibrateCommand
  testcalibratecommand.cpp
  testcalibratecommand.h)

add_pokit_app_unit_test(
  DeviceCommand
  testdevicecommand.cpp
  testdevicecommand.h)

add_pokit_app_unit_test(
  DsoCommand
  testdsocommand.cpp
  testdsocommand.h)

add_pokit_app_unit_test(
  FlashLedCommand
  testflashledcommand.cpp
  testflashledcommand.h)

add_pokit_app_unit_test(
  InfoCommand
  testinfocommand.cpp
  testinfocommand.h)

add_pokit_app_unit_test(
  LoggerFetchCommand
  testloggerfetchcommand.cpp
  testloggerfetchcommand.h)

add_pokit_app_unit_test(
  LoggerStartCommand
  testloggerstartcommand.cpp
  testloggerstartcommand.h)

add_pokit_app_unit_test(
  LoggerStopCommand
  testloggerstopcommand.cpp
  testloggerstopcommand.h)

add_pokit_app_unit_test(
  MeterCommand
  testmetercommand.cpp
  testmetercommand.h)

add_pokit_app_unit_test(
  ScanCommand
  testscancommand.cpp
  testscancommand.h)

add_pokit_app_unit_test(
  SetNameCommand
  testsetnamecommand.cpp
  testsetnamecommand.h)

add_pokit_app_unit_test(
  StatusCommand
  teststatuscommand.cpp
  teststatuscommand.h)
