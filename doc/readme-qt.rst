curecoin-qt: Qt5 GUI for Curecoin
================================

Build instructions
===================

##Debian
-------

First, make sure that the required packages for Qt5 development of your
distribution are installed, for Debian and Ubuntu these are:

This involves downloading the source, meeting the dependencies, compiling the code, and then installing the resulting software.

git clone https://github.com/cygnusxi/CurecoinSource.git

##FOR THE GUI CLIENT:

Ubuntu 22.04+ / Debian 12+:

sudo apt-get install qtbase5-dev qt5-qmake qtbase5-dev-tools qttools5-dev-tools libboost-dev libboost-system-dev libboost-filesystem-dev libboost-program-options-dev libboost-thread-dev libssl-dev libminiupnpc-dev libdb++-dev dh-make build-essential

Older Ubuntu 18.04 / Debian 10:

sudo apt-get install qt5-default qt5-qmake qtbase5-dev-tools qttools5-dev-tools libboost-dev libboost-system-dev libboost-filesystem-dev libboost-program-options-dev libboost-thread-dev libssl-dev libminiupnpc-dev libdb5.3++-dev dh-make build-essential

From the main directory, run the following:

qmake && make

sudo make install

Alternatively, don't run that command and just place the binary wherever you want.

##FOR THE HEADLESS CURECOIND:

Ubuntu 22.04+ / Debian 12+:

sudo apt-get install libboost-all-dev libqrencode-dev libssl-dev libdb++-dev libminiupnpc-dev dh-make build-essential

Older Ubuntu 18.04 / Debian 10:

sudo apt-get install libboost-all-dev libqrencode-dev libssl-dev libdb5.3-dev libdb5.3++-dev libminiupnpc-dev dh-make build-essential

cd src/ && mkdir obj/ && make -f makefile.unix

sudo make install

Alternatively, don't run that command and just place the binary wherever you want.

Note: Berkeley DB 5.3+ or 6.x are supported. The codebase now requires C++11 compiler support.

Alternatively, install `Qt Creator`_ and open the `curecoin-qt.pro` file.

An executable named `curecoin-qt` will be built.

.. _`Qt Creator`: http://qt.nokia.com/downloads/

##Windows
--------

Windows build instructions:

- Download the `Qt Windows SDK`_ and install it. You don't need the Symbian stuff, just the desktop Qt.

- Download and extract the `dependencies archive`_  [#]_, or compile openssl, boost and dbcxx yourself.

- Copy the contents of the folder "deps" to "X:\\QtSDK\\mingw", replace X:\\ with the location where you installed the Qt SDK. Make sure that the contents of "deps\\include" end up in the current "include" directory.

- Open the bitcoin-qt.pro file in Qt Creator and build as normal (ctrl-B)

.. _`Qt Windows SDK`: http://qt.nokia.com/downloads/sdk-windows-cpp
.. _`dependencies archive`: https://download.visucore.com/bitcoin/qtgui_deps_1.zip
.. [#] PGP signature: https://download.visucore.com/bitcoin/qtgui_deps_1.zip.sig (signed with RSA key ID `610945D0`_)
.. _`610945D0`: http://pgp.mit.edu:11371/pks/lookup?op=get&search=0x610945D0


##Mac OS X
--------

- Download and install the `Qt Mac OS X SDK`_. It is recommended to also install Apple's Xcode with UNIX tools.

- Download and install `MacPorts`_.

- Execute the following commands in a terminal to get the dependencies:

::

	sudo port selfupdate
	sudo port install boost db48 miniupnpc

- Open the bitcoin-qt.pro file in Qt Creator and build as normal (cmd-B)

.. _`Qt Mac OS X SDK`: http://qt.nokia.com/downloads/sdk-mac-os-cpp
.. _`MacPorts`: http://www.macports.org/install.php


Build configuration options
============================

UPnP port forwarding
---------------------

To use UPnP for port forwarding behind a NAT router (recommended, as more connections overall allow for a faster and more stable curecoin experience), pass the following argument to qmake:

::

    qmake "USE_UPNP=1"

(in **Qt Creator**, you can find the setting for additional qmake arguments under "Projects" -> "Build Settings" -> "Build Steps", then click "Details" next to **qmake**)

This requires miniupnpc for UPnP port mapping.  It can be downloaded from
http://miniupnp.tuxfamily.org/files/.  UPnP support is not compiled in by default.

Set USE_UPNP to a different value to control this:

+------------+--------------------------------------------------------------------------+
| USE_UPNP=- | no UPnP support, miniupnpc not required;                                 |
+------------+--------------------------------------------------------------------------+
| USE_UPNP=0 | (the default) built with UPnP, support turned off by default at runtime; |
+------------+--------------------------------------------------------------------------+
| USE_UPNP=1 | build with UPnP support turned on by default at runtime.                 |
+------------+--------------------------------------------------------------------------+

Notification support for recent (k)ubuntu versions
---------------------------------------------------

To see desktop notifications on (k)ubuntu versions starting from 10.04, enable usage of the
FreeDesktop notification interface through DBUS using the following qmake option:

::

    qmake "USE_DBUS=1"

Generation of QR codes
-----------------------

libqrencode may be used to generate QRCode images for payment requests. 
It can be downloaded from http://fukuchi.org/works/qrencode/index.html.en, or installed via your package manager. Pass the USE_QRCODE 
flag to qmake to control this:

+--------------+--------------------------------------------------------------------------+
| USE_QRCODE=0 | (the default) No QRCode support - libarcode not required                 |
+--------------+--------------------------------------------------------------------------+
| USE_QRCODE=1 | QRCode support enabled                                                   |
+--------------+--------------------------------------------------------------------------+


Berkeley DB version warning
==========================

A warning for people using the *static binary* version of curecoin on a Linux/UNIX-ish system (tl;dr: **Berkeley DB databases are not forward compatible**).

Older static binary versions of curecoin were linked against libdb4.8 (see also `this Debian issue`_).

Now the nasty thing is that databases from 5.X/6.X are not compatible with 4.X.

If the globally installed development package of Berkeley DB installed on your system is 5.X or 6.X, any source you
build yourself will be linked against that. The first time you run with a 5.X/6.X version the database will be upgraded,
and 4.X cannot open the new format. This means that you cannot go back to the old statically linked version without
significant hassle!

**Recommendation:** Use Berkeley DB 5.3+ or 6.x consistently. Modern builds (2024+) use libdb++ which links to the
latest available version on your system.

.. _`this Debian issue`: http://bugs.debian.org/cgi-bin/bugreport.cgi?bug=621425

Compiler Requirements
=====================

This codebase now requires C++11 support. Minimum compiler versions:

- GCC 7.0+
- Clang 5.0+
- MSVC 2017+

Recommended Library Versions (2024)
===================================

- Qt: 5.15 LTS or 6.x
- Boost: 1.70+ (tested up to 1.82+)
- OpenSSL: 1.1.1+ or 3.0+
- Berkeley DB: 5.3+ or 6.x

