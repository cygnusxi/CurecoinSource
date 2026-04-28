# Curecoin - Modern CMake Build System
# Compatible with curecoin-qt.pro and makefile.unix (legacy build files remain unchanged)
cmake_minimum_required(VERSION 3.16)
project(Curecoin VERSION 2.2.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)

# =============================================================================
# Build Options
# =============================================================================
option(BUILD_GUI "Build curecoin-qt GUI client" ON)
option(BUILD_DAEMON "Build curecoind headless daemon" ON)
option(USE_UPNP "Use UPnP for port mapping" ON)
option(USE_IPV6 "Enable IPv6 support" ON)
option(USE_QRCODE "Build with QR code support (libqrencode)" OFF)
option(USE_DBUS "Build with DBus support (Linux desktop notifications)" OFF)
option(USE_BUILD_INFO "Include git build info in version" ON)
option(STATIC "Static linking for daemon (Linux)" OFF)

# =============================================================================
# Find Dependencies
# =============================================================================
find_package(OpenSSL REQUIRED)
find_package(ZLIB REQUIRED)

# Berkeley DB
set(BDB_FOUND FALSE)
if(DEFINED ENV{BDB_PREFIX})
    set(BDB_ROOT $ENV{BDB_PREFIX})
endif()
find_path(BDB_INCLUDE_PATH db_cxx.h
    HINTS ${BDB_ROOT}
    PATH_SUFFIXES include include/db48 include/db4.8
)
find_library(BDB_LIBRARY NAMES db_cxx db_cxx-4.8 db_cxx-5.3
    HINTS ${BDB_ROOT}
    PATH_SUFFIXES lib lib64
)
if(BDB_INCLUDE_PATH AND BDB_LIBRARY)
    set(BDB_FOUND TRUE)
endif()
if(NOT BDB_FOUND)
    message(FATAL_ERROR "Berkeley DB not found. Set BDB_PREFIX or BDB_INCLUDE_PATH/BDB_LIBRARY")
endif()

# Boost
set(Boost_USE_STATIC_LIBS ${STATIC})
find_package(Boost 1.53 REQUIRED COMPONENTS system filesystem program_options thread)
if(WIN32)
    find_package(Boost 1.53 REQUIRED COMPONENTS chrono)
endif()

# ARM: skip SSE2 (Raspberry Pi)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "arm|aarch64|ARM")
    set(CURECOIN_SKIP_SSE2 TRUE)
else()
    set(CURECOIN_SKIP_SSE2 FALSE)
endif()

# MiniUPnP (optional for UPNP)
set(MINIUPNPC_FOUND FALSE)
if(USE_UPNP)
    find_path(MINIUPNPC_INCLUDE_DIR miniupnpc/miniupnpc.h
        HINTS $ENV{MINIUPNPC_ROOT} "C:/vcpkg/packages/miniupnpc_x64-mingw-static"
        PATH_SUFFIXES include
    )
    find_library(MINIUPNPC_LIBRARY NAMES miniupnpc
        HINTS $ENV{MINIUPNPC_ROOT} "C:/vcpkg/packages/miniupnpc_x64-mingw-static"
        PATH_SUFFIXES lib
    )
    if(MINIUPNPC_INCLUDE_DIR AND MINIUPNPC_LIBRARY)
        set(MINIUPNPC_FOUND TRUE)
    else()
        message(STATUS "MiniUPnP not found - building without UPNP")
        set(USE_UPNP OFF)
    endif()
endif()

# Qt5 (for GUI)
if(BUILD_GUI)
    find_package(Qt5 5.9 REQUIRED COMPONENTS Core Gui Network Widgets PrintSupport)
    if(USE_DBUS)
        find_package(Qt5 COMPONENTS DBus)
    endif()
    if(USE_QRCODE)
        find_package(PkgConfig)
        pkg_check_modules(QRENCODE libqrencode)
    endif()
endif()

# =============================================================================
# Common Definitions and Compiler Flags
# =============================================================================
set(CURECOIN_DEFINITIONS
    BOOST_THREAD_USE_LIB
    BOOST_SPIRIT_THREADSAFE
    BOOST_BIND_GLOBAL_PLACEHOLDERS
    __NO_SYSTEM_INCLUDES
)
if(WIN32)
    list(APPEND CURECOIN_DEFINITIONS
        BOOST_THREAD_PROVIDES_GENERIC_SHARED_MUTEX_ON_WIN
        BOOST_ASIO_ENABLE_OLD_SERVICES
        WIN32
    )
endif()
if(USE_UPNP)
    list(APPEND CURECOIN_DEFINITIONS USE_UPNP=1 MINIUPNP_STATICLIB)
endif()
if(USE_IPV6)
    list(APPEND CURECOIN_DEFINITIONS USE_IPV6=1)
endif()
if(USE_BUILD_INFO)
    list(APPEND CURECOIN_DEFINITIONS HAVE_BUILD_INFO)
endif()

# Build info header
set(BUILD_H_PATH "${CMAKE_BINARY_DIR}/build.h")
add_custom_command(OUTPUT "${BUILD_H_PATH}"
    COMMAND ${CMAKE_COMMAND} -P "${CMAKE_SOURCE_DIR}/cmake/genbuild.cmake"
            "${BUILD_H_PATH}" "${CMAKE_SOURCE_DIR}"
    COMMENT "Generating build.h"
    VERBATIM
)
add_custom_target(genbuild DEPENDS "${BUILD_H_PATH}")

# =============================================================================
# Core Library Sources (shared between daemon and GUI)
# =============================================================================
set(CORE_SOURCES
    src/alert.cpp
    src/version.cpp
    src/checkpoints.cpp
    src/netbase.cpp
    src/addrman.cpp
    src/crypter.cpp
    src/key.cpp
    src/db.cpp
    src/init.cpp
    src/keystore.cpp
    src/main.cpp
    src/net.cpp
    src/protocol.cpp
    src/curecoinrpc.cpp
    src/rpcdump.cpp
    src/rpcnet.cpp
    src/rpcmining.cpp
    src/rpcwallet.cpp
    src/rpcblockchain.cpp
    src/rpcrawtransaction.cpp
    src/script.cpp
    src/sync.cpp
    src/util.cpp
    src/wallet.cpp
    src/walletdb.cpp
    src/kernel.cpp
    src/json/json_spirit_value.cpp
    src/json/json_spirit_reader.cpp
    src/json/json_spirit_writer.cpp
)

# =============================================================================
# curecoind - Headless Daemon
# =============================================================================
if(BUILD_DAEMON)
    add_executable(curecoind
        ${CORE_SOURCES}
        src/noui.cpp
    )
    add_dependencies(curecoind genbuild)

    target_include_directories(curecoind PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/json
        ${CMAKE_BINARY_DIR}
        ${OPENSSL_INCLUDE_DIR}
        ${BDB_INCLUDE_PATH}
        ${Boost_INCLUDE_DIRS}
    )
    if(MINIUPNPC_FOUND)
        target_include_directories(curecoind PRIVATE ${MINIUPNPC_INCLUDE_DIR})
    endif()

    target_compile_definitions(curecoind PRIVATE ${CURECOIN_DEFINITIONS})
    target_compile_options(curecoind PRIVATE
        -Wall -Wextra -Wformat -Wformat-security -Wno-unused-parameter
        -Wno-deprecated-declarations
    )
    if(NOT WIN32 AND NOT APPLE AND NOT CURECOIN_SKIP_SSE2)
        target_compile_options(curecoind PRIVATE -msse2)
    endif()
    if(NOT WIN32 AND NOT APPLE)
        target_compile_options(curecoind PRIVATE
            -fstack-protector-all --param ssp-buffer-size=1
        )
        target_link_options(curecoind PRIVATE
            -Wl,-z,relro -Wl,-z,now -fstack-protector-all
        )
    endif()

    target_link_libraries(curecoind PRIVATE
        OpenSSL::SSL
        OpenSSL::Crypto
        ZLIB::ZLIB
        Boost::system
        Boost::filesystem
        Boost::program_options
        Boost::thread
        ${BDB_LIBRARY}
    )
    if(WIN32)
        target_link_libraries(curecoind PRIVATE Boost::chrono)
        target_link_libraries(curecoind PRIVATE
            ws2_32 shlwapi mswsock ole32 oleaut32 uuid gdi32 iphlpapi
        )
        target_link_options(curecoind PRIVATE -Wl,--dynamicbase -Wl,--nxcompat)
    else()
        target_link_libraries(curecoind PRIVATE rt dl pthread)
        target_compile_options(curecoind PRIVATE -pthread)
        target_link_options(curecoind PRIVATE -pthread)
    endif()
    if(MINIUPNPC_FOUND)
        target_link_libraries(curecoind PRIVATE ${MINIUPNPC_LIBRARY})
    endif()

    install(TARGETS curecoind RUNTIME DESTINATION bin)
endif()

# =============================================================================
# curecoin-qt - Qt GUI Client
# =============================================================================
if(BUILD_GUI)
    set(QT_SOURCES
        src/qt/curecoin.cpp
        src/qt/curecoingui.cpp
        src/qt/transactiontablemodel.cpp
        src/qt/addresstablemodel.cpp
        src/qt/optionsdialog.cpp
        src/qt/sendcoinsdialog.cpp
        src/qt/addressbookpage.cpp
        src/qt/signverifymessagedialog.cpp
        src/qt/aboutdialog.cpp
        src/qt/editaddressdialog.cpp
        src/qt/curecoinaddressvalidator.cpp
        src/qt/clientmodel.cpp
        src/qt/guiutil.cpp
        src/qt/transactionrecord.cpp
        src/qt/optionsmodel.cpp
        src/qt/monitoreddatamapper.cpp
        src/qt/transactiondesc.cpp
        src/qt/transactiondescdialog.cpp
        src/qt/curecoinstrings.cpp
        src/qt/curecoinamountfield.cpp
        src/qt/transactionfilterproxy.cpp
        src/qt/transactionview.cpp
        src/qt/walletmodel.cpp
        src/qt/overviewpage.cpp
        src/qt/csvmodelwriter.cpp
        src/qt/sendcoinsentry.cpp
        src/qt/qvalidatedlineedit.cpp
        src/qt/curecoinunits.cpp
        src/qt/qvaluecombobox.cpp
        src/qt/askpassphrasedialog.cpp
        src/qt/notificator.cpp
        src/qt/qtipcserver.cpp
        src/qt/rpcconsole.cpp
        src/noui.cpp
    )

    set(QT_FORMS
        src/qt/forms/sendcoinsdialog.ui
        src/qt/forms/addressbookpage.ui
        src/qt/forms/signverifymessagedialog.ui
        src/qt/forms/aboutdialog.ui
        src/qt/forms/editaddressdialog.ui
        src/qt/forms/transactiondescdialog.ui
        src/qt/forms/overviewpage.ui
        src/qt/forms/sendcoinsentry.ui
        src/qt/forms/askpassphrasedialog.ui
        src/qt/forms/rpcconsole.ui
        src/qt/forms/optionsdialog.ui
    )

    if(USE_QRCODE AND QRENCODE_FOUND)
        list(APPEND QT_SOURCES src/qt/qrcodedialog.cpp)
        list(APPEND QT_FORMS src/qt/forms/qrcodedialog.ui)
    endif()

    qt5_wrap_ui(QT_UI_HEADERS ${QT_FORMS})
    qt5_add_resources(QT_RCC_SOURCES src/qt/curecoin.qrc)

    add_executable(curecoin-qt
        ${CORE_SOURCES}
        ${QT_SOURCES}
        ${QT_UI_HEADERS}
        ${QT_RCC_SOURCES}
    )
    add_dependencies(curecoin-qt genbuild)

    target_include_directories(curecoin-qt PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/json
        ${CMAKE_SOURCE_DIR}/src/qt
        ${CMAKE_BINARY_DIR}
        ${OPENSSL_INCLUDE_DIR}
        ${BDB_INCLUDE_PATH}
        ${Boost_INCLUDE_DIRS}
    )
    if(MINIUPNPC_FOUND)
        target_include_directories(curecoin-qt PRIVATE ${MINIUPNPC_INCLUDE_DIR})
    endif()

    target_compile_definitions(curecoin-qt PRIVATE
        ${CURECOIN_DEFINITIONS}
        QT_GUI
        QT_DISABLE_DEPRECATED_BEFORE=0
    )
    if(USE_QRCODE AND QRENCODE_FOUND)
        target_compile_definitions(curecoin-qt PRIVATE USE_QRCODE)
    endif()
    if(USE_DBUS)
        target_compile_definitions(curecoin-qt PRIVATE USE_DBUS)
    endif()
    if(APPLE)
        target_compile_definitions(curecoin-qt PRIVATE MAC_OSX MSG_NOSIGNAL=0)
    endif()
    if(NOT WIN32 AND NOT APPLE)
        target_compile_definitions(curecoin-qt PRIVATE LINUX)
    endif()

    target_compile_options(curecoin-qt PRIVATE
        -Wall -Wextra -Wformat -Wformat-security -Wno-unused-parameter
        -Wno-deprecated-declarations
    )
    if(NOT WIN32 AND NOT APPLE AND NOT CURECOIN_SKIP_SSE2)
        target_compile_options(curecoin-qt PRIVATE -msse2)
    endif()
    if(NOT WIN32 AND NOT APPLE)
        target_compile_options(curecoin-qt PRIVATE
            -fstack-protector-all --param ssp-buffer-size=1
        )
        target_link_options(curecoin-qt PRIVATE
            -Wl,-z,relro -Wl,-z,now -fstack-protector-all
        )
    endif()
    if(WIN32)
        target_link_options(curecoin-qt PRIVATE -Wl,--dynamicbase -Wl,--nxcompat)
    endif()

    target_link_libraries(curecoin-qt PRIVATE
        Qt5::Core
        Qt5::Gui
        Qt5::Network
        Qt5::Widgets
        Qt5::PrintSupport
        OpenSSL::SSL
        OpenSSL::Crypto
        ZLIB::ZLIB
        Boost::system
        Boost::filesystem
        Boost::program_options
        Boost::thread
        ${BDB_LIBRARY}
    )
    if(WIN32)
        target_link_libraries(curecoin-qt PRIVATE Boost::chrono)
        target_link_libraries(curecoin-qt PRIVATE
            ws2_32 shlwapi mswsock ole32 oleaut32 uuid gdi32 iphlpapi
        )
    endif()
    if(NOT WIN32 AND NOT APPLE)
        target_link_libraries(curecoin-qt PRIVATE rt)
        target_compile_options(curecoin-qt PRIVATE -pthread)
        target_link_options(curecoin-qt PRIVATE -pthread)
    endif()
    if(APPLE)
        target_link_libraries(curecoin-qt PRIVATE
            "-framework Foundation"
            "-framework ApplicationServices"
            "-framework AppKit"
        )
        target_sources(curecoin-qt PRIVATE src/qt/macdockiconhandler.mm)
    endif()
    if(USE_DBUS)
        target_link_libraries(curecoin-qt PRIVATE Qt5::DBus)
    endif()
    if(USE_QRCODE AND QRENCODE_FOUND)
        target_link_libraries(curecoin-qt PRIVATE ${QRENCODE_LIBRARIES})
        target_include_directories(curecoin-qt PRIVATE ${QRENCODE_INCLUDE_DIRS})
    endif()
    if(MINIUPNPC_FOUND)
        target_link_libraries(curecoin-qt PRIVATE ${MINIUPNPC_LIBRARY})
    endif()

    # Windows: embed version info via RC file
    if(WIN32)
        target_sources(curecoin-qt PRIVATE src/qt/res/curecoin-qt.rc)
    endif()

    install(TARGETS curecoin-qt RUNTIME DESTINATION bin)
endif()

# =============================================================================
# Summary
# =============================================================================
message(STATUS "")
message(STATUS "Curecoin build configuration:")
message(STATUS "  Build daemon (curecoind): ${BUILD_DAEMON}")
message(STATUS "  Build GUI (curecoin-qt):  ${BUILD_GUI}")
message(STATUS "  USE_UPNP:   ${USE_UPNP}")
message(STATUS "  USE_IPV6:   ${USE_IPV6}")
message(STATUS "  USE_QRCODE: ${USE_QRCODE}")
message(STATUS "")
