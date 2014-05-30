#Curecoin Client
###Client for the Curecoin cryptocurrency

### New to Curecoin?

If you are a newcomer, check out this [this website](http://curecoin.net/) which explains Curecoin in simple and everyday terms. You can find statistics on the current state of the Curecoin network [on this page](http://stats.curecoinfolding.com/). Joining the Curecoin network is easy: once you've installed the Folding@home software, pick a username, [get a passkey](http://folding.stanford.edu/home/faq/faq-passkey/), and enter 224497 as the team number to fold under. Then register on [cryptobullionpools.com](https://www.cryptobullionpools.com/) with the **exact** same username, and within a day you should get some coins!

### System requirements

1. The Linux operating system.

### Installation

There are two Curecoin clients that you can choose: one with a nice graphical interface, and one that operates entirely in the command-line. The first is highly recommended and is a good choice for most users, but expert users may prefer the command-line (headless) client instead, so I offer both. The GUI client is known as "curecoin-qt" and the headless client is called "curecoind". Installing either one or both is extremely simple. Just follow these directions:

* **If you are a Ubuntu, Xubuntu, Kubuntu, or Linux Mint user or if you run [any other](http://tinyurl.com/ubuntu-derivatives) Ubuntu-based system, please install from my PPA.** This is very easy to do as you simply run these commands:
> 1. **sudo add-apt-repository ppa:jvictors/gitstable**
> 2. **sudo apt-get update**
> 3. **sudo apt-get install curecoin-qt**
> 4. (or for the headless client, **sudo apt-get install curecoind**)

    This will add my Personal Package Archive (PPA) to your sources, refresh the list of available packages, and then install the Curecoin client. Digital signatures and hashsums provide authentication during publishing and ensure integrity during the installation. Once installed, the client can then be managed, auto-updated, and uninstalled just like any other Linux package. This makes it easy to stay updated on the latest releases.

* **If you are a Debian Wheezy user or use any other Debian-based system, please install from a .deb file.**
> 1. Visit the [Releases page](https://github.com/Jesse-V/CurecoinSource/releases).
> 2. Download the .deb file for the latest release. i386 is 32-bit, amd64 is 64-bit.
> 3. Install the .deb package with **dpkg -i**

    Debian Wheezy is the only version supported at the moment. Due to dependency issues, Debian users will not be able to install .deb files built for Ubuntu releases, and visa-versa. i386 and amd64 are the only two architectures currently packaged, so Raspberry Pi users must compile from source at the moment, although I am currently working on providing ARM builds for you guys.

* **Install from source**

    If you run any other flavor of Linux not mentioned above, or you feel that you are hardcore and feel the need to compile your own software, choose this option. This involves downloading the source, meeting the dependencies, compiling the code, and then installing the resulting software. Fortunately, this is easy enough for both the GUI QT-based client and the headless CLI curecoind.

    There are a number of options by which you can get the source. The first and recommended approach is **git clone https://github.com/Jesse-V/CurecoinSource.git**, but if you want a .zip, use [this link](https://github.com/Jesse-V/CurecoinSource/archive/master.zip). The second option is to visit the [Releases page](https://github.com/Jesse-V/CurecoinSource/releases) and downloading the .zip source of the latest stable edition.

    FOR THE GUI CLIENT:

    > 1. **sudo apt-get --no-install-recommends install qt4-qmake libqt4-dev libboost-dev libboost-system-dev libboost-filesystem-dev libboost-program-options-dev libboost-thread-dev libssl-dev libminiupnpc-dev libdb5.1++-dev dh-make build-essential**
    > 2. From the main directory, run the following:
    > 3. **qmake && make**
    > 4. **sudo make install** Alternatively, don't run that command and just place the binary wherever you want.

    FOR THE HEADLESS CURECOIND:

    > 1. **sudo apt-get --no-install-recommends install libboost-all-dev libqrencode-dev libssl-dev libdb5.1-dev libdb5.1++-dev libminiupnpc-dev dh-make build-essential**
    > 2. **cd src/ && mkdir obj/ && make -f makefile.unix**
    > 4. **sudo make install** Alternatively, don't run that command and just place the binary wherever you want.

    libdb4.8 should also work if libdb5.1 is too high a version for you.

### Reporting bugs or getting assistance

Since I package and maintain the Linux Curecoin clients, I'd appreciate reports if you encounter problems related to the compiling, packaging, or installation of the client. You can find me as an OP on Freenode's #curecoin IRC channel, you can open an issue ticket in this repo, or you can send me an email, and I'll do what I can to help.

General help for Curecoin can be found on the forums at http://www.curecoin.us/ or on #curecoin. I've found the community to be both enthusiastic and willing to assist so ask around, but if you have a highly technical question Cygnus-XI and vorksholk are both extremely knowledgeable and may be able to help in greater detail.

### Donations

If you have a Github account, I'd very much appreciate stars on this repo as it helps improve my profile and increases the visibility of this repository. If you'd like to do more than that, tip cryptocurrency to:

Jesse V - Maintaines linux builds

Curecoin wallet: BJEWt96yDpAo8HFwnmijfvb1JkT8axSoe1

Bitcoin wallet: 1M2hoFPhiK8J4Da2mZ6JuqKT2Hh8KRQ77g

![](http://tuxnet-opti980.main.usu.edu/unlistedDir/pics/BTC_addr.png "QR code of my Bitcoin address")

CygnusXI - Curecoin Dev

Curecoin wallet B4nMZWxPs7mjUacHYf5wmtEEd489UpnExd

Bitcoin wallet: 1G1Sac4sJXqTu2ZhPJFH86HVqs6YKFD4MW

VorkSholk - Curecoin Dev

Curecoin wallet not yet listed
