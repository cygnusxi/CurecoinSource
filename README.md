#Curecoin Client
###Client for the Curecoin cryptocurrency

### New to Curecoin?

If you are a newcomer, check out this [this website](https://curecoin.net/) which explains Curecoin in simple and everyday terms. You can find statistics on the current state of the Curecoin network [on this page](http://folding.extremeoverclocking.com/team_summary.php?s&t=224497). Joining the Curecoin network is easy: once you've installed the Folding@home software, pick a username, [get a passkey](http://folding.stanford.edu/home/faq/faq-passkey/), and enter 224497 as the team number to fold under. Then register on [cryptobullionpools.com](https://www.cryptobullionpools.com/) with the **exact** same username (case sensitive), and within a day you should get some coins!

### System requirements

1. The Linux operating system.

### Installation

There are two Curecoin clients that you can choose: one with a nice graphical interface, and one that operates entirely in the command-line. The first is highly recommended and is a good choice for most users, but expert users may prefer the command-line (headless) client instead. The GUI client is known as "curecoin-qt" and the headless client is called "curecoind". Installing either one or both is extremely simple. Just follow these directions:

* **Install from source**

  This involves downloading the source, meeting the dependencies, compiling the code, and then installing the resulting software. 

   git clone https://github.com/cygnusxi/CurecoinSource.git

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

General help for Curecoin can be found on the forums at https://www.curecoin.net/forum or on #curecoin in irc.freenode. The forum is recommended for non frequent users of irc. 

Jesse V packaged the (now outdated) Linux Curecoin binaries, if you encounter problems related use of the client you can submit a bug report at https://github.com/Jesse-V/CurecoinSource .

### Donations

Stars on this repo are appreciated as it helps improve the visibility of this repository. If you'd like to do more than that, tip cryptocurrency to:


CygnusXI - Curecoin Dev

Curecoin wallet B4nMZWxPs7mjUacHYf5wmtEEd489UpnExd

Bitcoin wallet: 1G1Sac4sJXqTu2ZhPJFH86HVqs6YKFD4MW
