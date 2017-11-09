## CureCoin Wallet 

### Overview 

If you are a newcomer, check out this [this website](http://curecoin.net/) which explains Curecoin in simple and everyday terms. You can find statistics on the current state of the Curecoin research rates [on this page](http://folding.extremeoverclocking.com/team_summary.php?s=&t=224497) and blockchain status and info on the [Block Explorer](https://chainz.cryptoid.info/cure/)

Joining the Curecoin network is easy:

1. Install the [Folding@home software](http://folding.stanford.edu/)
2. Pick a username & get a [passkey](http://folding.stanford.edu/home/faq/faq-passkey/)
3. Enter "224497" as the team number to fold under
4. Register on [cryptobullionpools.com](https://www.cryptobullionpools.com/) with the **exact** same username 
5. Expect your coins within 24 hours!

### Standard Installation

1. To see all version for Windows, Mac, Linux wallet downloads visit [curecoin releases](https://github.com/cygnusxi/CurecoinSource/releases)
2. [Windows Download](https://github.com/cygnusxi/CurecoinSource/releases/tag/v1.9.4.1)
3. [Mac Download](https://github.com/cygnusxi/CurecoinSource/releases/tag/v1.9.2.1)

### Installation for Linux from source

If you do not use Linux see previous section for a prebuilt wallet. There are two CureCoin Linux based clients that you can compile for yourself: one with a nice graphical interface, and one that operates entirely in the command-line. The first is highly recommended and is a good choice for most users, but expert users may prefer the command-line (headless) client instead. The GUI client is known as "curecoin-qt" and the headless client is called "curecoind". Installing either one or both is extremely simple. Just follow these directions:

* **Install from source**

  This involves downloading the source, meeting the dependencies, compiling the code, and then installing the resulting software. 

   git clone https://github.com/cygnusxi/CurecoinSource.git

    FOR THE GUI CLIENT:

    > 1. **sudo apt-get install qt5-default qt5-qmake qtbase5-dev-tools qttools5-dev-tools libboost-dev libboost-system-dev libboost-filesystem-dev libboost-program-options-dev libboost-thread-dev libssl-dev libminiupnpc-dev libdb5.3++-dev dh-make build-essential**
    > 2. From the main directory, run the following:
    > 3. **qmake && make**

    FOR THE HEADLESS CURECOIND:

    > 1. **sudo apt-get install libboost-all-dev libqrencode-dev libssl-dev libdb5.3-dev libdb5.3++-dev libminiupnpc-dev dh-make build-essential**
    > 2. **cd src**
    > 3. **make -f makefile.unix**
    > 4. **sudo make install** Alternatively, don't run that command and just place the binary wherever you want.

    libdb4.8 should also work if libdb5.1 is too high a version for you. Newer versions of Linux will need libdb5.3++-dev

### Reporting bugs or getting assistance

General help for CureCoin can be found on the forums at https://www.curecoin.net/forum or on Slack http://slack.curecoin.net:30000/ . The forum is recommended for non frequent users of irc. 

### Donations

Stars on this repo are appreciated as it helps improve the visibility of this repository. If you'd like to do more than that you can tip curecoins on the curecoin.net homepage. 
