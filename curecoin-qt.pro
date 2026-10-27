TEMPLATE = app
TARGET = curecoin-qt
VERSION = 2.5.3.2
INCLUDEPATH += src src/json src/qt

# Modernized C++ Standard for newer GCC/OS support
CONFIG += no_include_pwd c++17
QT += core gui network dbus

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets printsupport
    DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0
}

# --- BRANDED BUILD OUTPUT ---
!build_pass {
    message(" ")
    message("=======================================================")
    message("       🧬  CURECOIN QT-WALLET BUILD SYSTEM  🧬        ")
    message("=======================================================")
    message("  -> Target OS:       $$system(uname -s)")
    message("  -> Architecture:    $$QMAKE_HOST.arch")
    message("  -> Qt Version:      $$QT_VERSION")
    message("  -> UPNP Enabled:    $$USE_UPNP")
    message("  -> IPv6 Enabled:    $$USE_IPV6")
    message("-------------------------------------------------------")

    win32 {
        message("  [Windows / MSYS2 Build Environment Detected]")
        message("  -> BDB 5.3 Path:    C:/deps/db5")
        message("  -> Compiler:        MinGW-w64 (g++)")
    }

    linux {
        message("  [Linux Build Environment Detected]")
        message("  -> BDB 5.3 Path:    System Default or /usr/local")
        message("  -> Boost Modules:   Header-only (1.90+ compatible)")
    }

    macx {
        message("  [macOS Build Environment Detected]")
        message("  -> BDB 5.3 Path:    Homebrew (/opt/homebrew/opt/berkeley-db@5)")
        message("  -> Platform:        $$QMAKE_MAC_SDK")
    }

    message("=======================================================")
    message(" ")
}
# ----------------------------

# Global Defines
DEFINES += QT_GUI BOOST_THREAD_USE_LIB BOOST_SPIRIT_THREADSAFE BOOST_THREAD_PROVIDES_GENERIC_SHARED_MUTEX_ON_WIN __NO_SYSTEM_INCLUDES BOOST_BIND_GLOBAL_PLACEHOLDERS BOOST_ASIO_ENABLE_OLD_SERVICES USE_UPNP=1

# --- VCPKG Integration (Windows) ---
win32 {
    DEFINES += MINIUPNP_STATICLIB
    VCPKG_ROOT = C:/vcpkg/packages/miniupnpc_x64-mingw-static
    INCLUDEPATH += $$VCPKG_ROOT/include
    LIBS += -L$$VCPKG_ROOT/lib -lminiupnpc -liphlpapi
}
!win32 {
    LIBS += -lminiupnpc
}
# -------------------------

OBJECTS_DIR = build
MOC_DIR = build
UI_DIR = build

# use: qmake "RELEASE=1"
contains(RELEASE, 1) {
    macx:QMAKE_CXXFLAGS += -mmacosx-version-min=10.13

    !windows:!macx {
        !contains(QT_ARCH, arm):!contains(QT_ARCH, arm64) {
            LIBS += -Wl,-Bstatic
        }
    }
}

!win32 {
    QMAKE_CXXFLAGS *= -fstack-protector-all --param ssp-buffer-size=1
    QMAKE_LFLAGS *= -fstack-protector-all --param ssp-buffer-size=1
}

win32:QMAKE_LFLAGS *= -Wl,--dynamicbase -Wl,--nxcompat

contains(USE_QRCODE, 1) {
    message(Building with QRCode support)
    DEFINES += USE_QRCODE
    LIBS += -lqrencode -lpthread
}

contains(USE_DBUS, 1) {
    message(Building with DBUS (Freedesktop notifications) support)
    DEFINES += USE_DBUS
}

contains(USE_IPV6, -) {
    message(Building without IPv6 support)
} else {
    count(USE_IPV6, 0) {
        USE_IPV6=1
    }
    DEFINES += USE_IPV6=$$USE_IPV6
}

contains(curecoin_NEED_QT_PLUGINS, 1) {
    DEFINES += curecoin_NEED_QT_PLUGINS
    QTPLUGIN += qcncodecs qjpcodecs qtwcodecs qkrcodecs qtaccessiblewidgets
}

!windows|contains(USE_BUILD_INFO, 1) {
    genbuild.depends = FORCE
    genbuild.commands = cd $$PWD; /bin/sh share/genbuild.sh $$OUT_PWD/build/build.h
    genbuild.target = $$OUT_PWD/build/build.h
    PRE_TARGETDEPS += $$OUT_PWD/build/build.h
    QMAKE_EXTRA_TARGETS += genbuild
    DEFINES += HAVE_BUILD_INFO
}

!contains(QT_ARCH, arm):!contains(QT_ARCH, arm64) {
    QMAKE_CXXFLAGS += -msse2
    QMAKE_CFLAGS += -msse2
}

QMAKE_CXXFLAGS_WARN_ON = -fdiagnostics-show-option -Wall -Wextra -Wformat -Wformat-security -Wno-unused-parameter -Wstack-protector
QMAKE_CXXFLAGS += -Wno-deprecated-declarations

DEPENDPATH += src src/json src/qt
HEADERS += src/qt/curecoingui.h \
    src/qt/transactiontablemodel.h \
    src/qt/addresstablemodel.h \
    src/qt/optionsdialog.h \
    src/qt/sendcoinsdialog.h \
    src/qt/addressbookpage.h \
    src/qt/signverifymessagedialog.h \
    src/qt/aboutdialog.h \
    src/qt/editaddressdialog.h \
    src/qt/curecoinaddressvalidator.h \
    src/alert.h \
    src/addrman.h \
    src/base58.h \
    src/bignum.h \
    src/checkpoints.h \
    src/compat.h \
    src/sync.h \
    src/util.h \
    src/uint256.h \
    src/kernel.h \
    src/serialize.h \
    src/strlcpy.h \
    src/main.h \
    src/net.h \
    src/key.h \
    src/db.h \
    src/walletdb.h \
    src/script.h \
    src/init.h \
    src/mruset.h \
    src/json/json_spirit_writer_template.h \
    src/json/json_spirit_writer.h \
    src/json/json_spirit_value.h \
    src/json/json_spirit_utils.h \
    src/json/json_spirit_stream_reader.h \
    src/json/json_spirit_reader_template.h \
    src/json/json_spirit_reader.h \
    src/json/json_spirit_error_position.h \
    src/json/json_spirit.h \
    src/qt/clientmodel.h \
    src/qt/guiutil.h \
    src/qt/transactionrecord.h \
    src/qt/guiconstants.h \
    src/qt/optionsmodel.h \
    src/qt/monitoreddatamapper.h \
    src/qt/transactiondesc.h \
    src/qt/transactiondescdialog.h \
    src/qt/curecoinamountfield.h \
    src/wallet.h \
    src/keystore.h \
    src/qt/transactionfilterproxy.h \
    src/qt/transactionview.h \
    src/qt/walletmodel.h \
    src/curecoinrpc.h \
    src/qt/overviewpage.h \
    src/qt/csvmodelwriter.h \
    src/crypter.h \
    src/qt/sendcoinsentry.h \
    src/qt/qvalidatedlineedit.h \
    src/qt/curecoinunits.h \
    src/qt/qvaluecombobox.h \
    src/qt/askpassphrasedialog.h \
    src/protocol.h \
    src/qt/notificator.h \
    src/qt/qtipcserver.h \
    src/allocators.h \
    src/ui_interface.h \
    src/qt/rpcconsole.h \
    src/qt/trafficgraphwidget.h \
    src/qt/peertablemodel.h \
    src/version.h \
    src/netbase.h \
    src/clientversion.h

SOURCES += src/qt/curecoin.cpp src/qt/curecoingui.cpp \
    src/qt/transactiontablemodel.cpp \
    src/qt/addresstablemodel.cpp \
    src/qt/optionsdialog.cpp \
    src/qt/sendcoinsdialog.cpp \
    src/qt/addressbookpage.cpp \
    src/qt/signverifymessagedialog.cpp \
    src/qt/aboutdialog.cpp \
    src/qt/editaddressdialog.cpp \
    src/qt/curecoinaddressvalidator.cpp \
    src/alert.cpp \
    src/version.cpp \
    src/sync.cpp \
    src/util.cpp \
    src/netbase.cpp \
    src/key.cpp \
    src/script.cpp \
    src/main.cpp \
    src/init.cpp \
    src/net.cpp \
    src/checkpoints.cpp \
    src/addrman.cpp \
    src/db.cpp \
    src/walletdb.cpp \
    src/qt/clientmodel.cpp \
    src/qt/guiutil.cpp \
    src/qt/transactionrecord.cpp \
    src/qt/optionsmodel.cpp \
    src/qt/monitoreddatamapper.cpp \
    src/qt/transactiondesc.cpp \
    src/qt/transactiondescdialog.cpp \
    src/qt/curecoinstrings.cpp \
    src/qt/curecoinamountfield.cpp \
    src/wallet.cpp \
    src/keystore.cpp \
    src/qt/transactionfilterproxy.cpp \
    src/qt/transactionview.cpp \
    src/qt/walletmodel.cpp \
    src/curecoinrpc.cpp \
    src/rpcdump.cpp \
    src/rpcnet.cpp \
    src/rpcmining.cpp \
    src/rpcwallet.cpp \
    src/rpcblockchain.cpp \
    src/rpcrawtransaction.cpp \
    src/qt/overviewpage.cpp \
    src/qt/csvmodelwriter.cpp \
    src/crypter.cpp \
    src/qt/sendcoinsentry.cpp \
    src/qt/qvalidatedlineedit.cpp \
    src/qt/curecoinunits.cpp \
    src/qt/qvaluecombobox.cpp \
    src/qt/askpassphrasedialog.cpp \
    src/protocol.cpp \
    src/qt/notificator.cpp \
    src/qt/qtipcserver.cpp \
    src/qt/rpcconsole.cpp \
    src/qt/trafficgraphwidget.cpp \
    src/qt/peertablemodel.cpp \
    src/noui.cpp \
    src/kernel.cpp

RESOURCES += \
    src/qt/curecoin.qrc

FORMS += \
    src/qt/forms/sendcoinsdialog.ui \
    src/qt/forms/addressbookpage.ui \
    src/qt/forms/signverifymessagedialog.ui \
    src/qt/forms/aboutdialog.ui \
    src/qt/forms/editaddressdialog.ui \
    src/qt/forms/transactiondescdialog.ui \
    src/qt/forms/overviewpage.ui \
    src/qt/forms/sendcoinsentry.ui \
    src/qt/forms/askpassphrasedialog.ui \
    src/qt/forms/rpcconsole.ui \
    src/qt/forms/optionsdialog.ui

contains(USE_QRCODE, 1) {
    HEADERS += src/qt/qrcodedialog.h
    SOURCES += src/qt/qrcodedialog.cpp
    FORMS += src/qt/forms/qrcodedialog.ui
}

contains(curecoin_QT_TEST, 1) {
    SOURCES += src/qt/test/test_main.cpp \
        src/qt/test/uritests.cpp
    HEADERS += src/qt/test/uritests.h
    DEPENDPATH += src/qt/test
    QT += testlib
    TARGET = curecoin-qt_test
    DEFINES += curecoin_QT_TEST
}

# --- TRANSLATION COMPILER (Modernized & Silenced) ---
TRANSLATIONS = $$files(src/qt/locale/curecoin_*.ts)

isEmpty(QMAKE_LRELEASE) {
    win32:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]\\lrelease.exe
    else:QMAKE_LRELEASE = $$[QT_INSTALL_BINS]/lrelease
}
isEmpty(QM_DIR):QM_DIR = $$PWD/src/qt/locale

TSQM.name = lrelease ${QMAKE_FILE_IN}
TSQM.input = TRANSLATIONS
TSQM.output = $$QM_DIR/${QMAKE_FILE_BASE}.qm
TSQM.commands = $$QMAKE_LRELEASE -silent ${QMAKE_FILE_IN} -qm ${QMAKE_FILE_OUT}
TSQM.CONFIG = no_link
QMAKE_EXTRA_COMPILERS += TSQM
# ----------------------------------------------------

OTHER_FILES += \
    doc/*.rst doc/*.txt doc/README README.md res/curecoin-qt.rc src/test/*.cpp src/test/*.h src/qt/test/*.cpp src/qt/test/*.h

isEmpty(BOOST_LIB_SUFFIX) {
    macx:BOOST_LIB_SUFFIX = -mt
    win32:BOOST_LIB_SUFFIX = -mt
}

isEmpty(BOOST_THREAD_LIB_SUFFIX) {
    BOOST_THREAD_LIB_SUFFIX = $$BOOST_LIB_SUFFIX
}

isEmpty(BDB_LIB_PATH)|isEmpty(BDB_INCLUDE_PATH) {
    macx {
        equals(QMAKE_HOST.arch, arm64) {
            isEmpty(BDB_LIB_PATH): BDB_LIB_PATH = /opt/homebrew/opt/berkeley-db@5/lib
            isEmpty(BDB_INCLUDE_PATH): BDB_INCLUDE_PATH = /opt/homebrew/opt/berkeley-db@5/include
        } else {
            isEmpty(BDB_LIB_PATH): BDB_LIB_PATH = /usr/local/opt/berkeley-db@5/lib
            isEmpty(BDB_INCLUDE_PATH): BDB_INCLUDE_PATH = /usr/local/opt/berkeley-db@5/include
        }
    }
}

win32 {
    BDB_INCLUDE_PATH = C:/deps/db5/include
    BDB_LIB_PATH = C:/deps/db5/lib
}

isEmpty(BOOST_LIB_PATH) {
    macx:BOOST_LIB_PATH = /opt/local/lib
}

isEmpty(BOOST_INCLUDE_PATH) {
    macx:BOOST_INCLUDE_PATH = /opt/local/include
}

windows:DEFINES += WIN32
windows:RC_FILE = src/qt/res/curecoin-qt.rc

windows:!contains(MINGW_THREAD_BUGFIX, 0) {
    DEFINES += _MT
    QMAKE_LIBS_QT_ENTRY = -lmingwthrd $$QMAKE_LIBS_QT_ENTRY
}

!windows:!macx {
    DEFINES += LINUX
    LIBS += -lrt
}

macx:HEADERS += src/qt/macdockiconhandler.h
macx:OBJECTIVE_SOURCES += src/qt/macdockiconhandler.mm
macx:LIBS += -framework Foundation -framework ApplicationServices -framework AppKit
macx:DEFINES += MAC_OSX MSG_NOSIGNAL=0
macx:ICON = src/qt/res/icons/curecoin.icns
macx:TARGET = "curecoin-Qt"
macx:QMAKE_CFLAGS_THREAD += -pthread
macx:QMAKE_LFLAGS_THREAD += -pthread
macx:QMAKE_CXXFLAGS_THREAD += -pthread

INCLUDEPATH += $$BOOST_INCLUDE_PATH $$BDB_INCLUDE_PATH $$OPENSSL_INCLUDE_PATH $$QRENCODE_INCLUDE_PATH
LIBS += $$join(BOOST_LIB_PATH,,-L,) $$join(BDB_LIB_PATH,,-L,) $$join(OPENSSL_LIB_PATH,,-L,) $$join(QRENCODE_LIB_PATH,,-L,)
LIBS += -lssl -lcrypto -ldb_cxx
windows:LIBS += -lws2_32 -lshlwapi -lmswsock -lole32 -loleaut32 -luuid -lgdi32

contains(QT_ARCH, arm)|contains(QT_ARCH, arm64) {
    LIBS += -lboost_filesystem$$BOOST_LIB_SUFFIX -lboost_program_options$$BOOST_LIB_SUFFIX -lboost_thread$$BOOST_THREAD_LIB_SUFFIX
} else {
    LIBS += -Wl,-Bstatic -lboost_filesystem$$BOOST_LIB_SUFFIX -lboost_program_options$$BOOST_LIB_SUFFIX -lboost_thread$$BOOST_THREAD_LIB_SUFFIX -Wl,-Bdynamic
}

windows:LIBS += -lboost_chrono$$BOOST_LIB_SUFFIX

contains(RELEASE, 1) {
    !windows:!macx {
        LIBS += -Wl,-Bdynamic
    }
}