## CureCoin Wallet 

### Overview 

If you are a newcomer, check out the [CureCoin website](https://curecoin.net/) which explains CureCoin in simple and everyday terms. You can find statistics on the current state of the CureCoin research rates on the [Team Stats Page](https://folding.extremeoverclocking.com/team_summary.php?s=&t=224497) and blockchain status and info on the [CureCoin Block Explorer](https://chainz.cryptoid.info/cure/)

### Joining the CureCoin network is easy:
On Windows, the [All-In-One FoldingBrowser Installer](https://github.com/Hou5e/FoldingBrowser/releases) provides a quick setup for running Folding@home to earn both CureCoin and FoldingCoin.

### Or, setup for getting CureCoin only for running Folding@home with any username:
1. Install the [Folding@home software](https://foldingathome.org/start-folding/)
2. Pick a Folding@home Username & get a [Passkey](https://apps.foldingathome.org/getpasskey)
3. Enter `224497` as the Folding@home Team number to fold under
4. Register on [Curecoin Research Core website](https://researchcore.curecoin.net/) with the **exact** same username (50 characters or less) used in Folding@home
5. Complete some Folding@home Work Units, and expect your CureCoins within 24-32 hours!

## Standard Wallet Installation

[CureCoin GitHub Releases](https://github.com/cygnusxi/CurecoinSource/releases) has the Windows, Mac, Linux wallet downloads.  
**CureCoin v2.0 is a mandatory update.** Older wallets need to be updated to this version (or newer) to work correctly.

NOTE: The blockchain, `wallet.dat`, and other settings like the `curecoin.conf` file go in the hidden folders at:
- Windows: `C:\Users\YourUser\AppData\Roaming\curecoin`
- Linux: `~/.curecoin`
- Mac: `~/Library/Application Support/curecoin`

If the wallet is not syncing immediately, just be patient. Let the wallet run for a day, and will sync up with the network once it finds other nodes to connect to. If the wallet is not connecting to other nodes:
- You may need to Port Forward port `9911` on your router to the wallet PC.
- You can add additional seed nodes in a `curecoin.conf` file, from [Network 'Node List' Buttons](https://chainz.cryptoid.info/cure/#!network), like:
  ```
  addnode=111.111.111.111
  addnode=222.222.222.222
  ```
## Linux Installation From Source

If you do not use Linux, see previous section for a pre-built wallet. There are two CureCoin Linux based clients that you can compile for yourself: one with a nice graphical interface, and one that operates entirely in the command-line. The first is highly recommended and is a good choice for most users, but expert users may prefer the command-line (headless) client instead. The GUI client is known as "curecoin-qt" and the headless client is called "curecoind". Installing either one or both is extremely simple. Just follow these directions:

### INSTALL STEPS FOR LINUX:

This involves downloading the source, meeting the dependencies, compiling the code, and then installing the resulting software.
- Install Git, if needed: `sudo apt install git`
- Get the source code: `git clone https://github.com/cygnusxi/CurecoinSource.git`

### FOR GUI CLIENT (curecoin-qt):
1. If `qt5-default` gives this command an error, then remove it and optionally add `qtbase5-dev` for newer versions of Linux: `sudo apt-get install qtcreator qt5-default qt5-qmake qtbase5-dev-tools qttools5-dev-tools cmake libboost-dev libboost-system-dev libboost-filesystem-dev libboost-program-options-dev libboost-thread-dev libssl-dev libminiupnpc-dev libdb5.3++-dev dh-make build-essential`
2. From the main directory `cd CurecoinSource`, run the following:
3. `qmake && make`
4. Run the GUI wallet with: `./curecoin-qt`

### FOR HEADLESS (curecoind):
1. `sudo apt-get install libboost-all-dev libqrencode-dev libssl-dev libdb5.3-dev libdb5.3++-dev libminiupnpc-dev dh-make build-essential`
2. `cd src`
3. `make -f makefile.unix`
4. `sudo make install` &nbsp; &nbsp; {Alternatively, don't run that command, and just place the binary wherever you want}
5. Run the command line only wallet with: `./curecoind`
   * To check status, use in a separate terminal: `watch "./curecoind getinfo"` or for a quick glimpse: `./curecoind getinfo`
   * See the full list of commands with: `./curecoind help`
   * The wallet and blockchain files will be stored in: `~/.curecoin`

### GUI Wallet Linux Example: Install Steps on Mint 21.1 / Ubuntu 22.04:
```
sudo apt install git
sudo apt-get install qtcreator qtbase5-dev qt5-qmake qtbase5-dev-tools qttools5-dev-tools cmake libboost-dev libboost-system-dev libboost-filesystem-dev libboost-program-options-dev libboost-thread-dev libssl-dev libminiupnpc-dev libdb5.3++-dev dh-make build-essential

mkdir curecoin
cd curecoin

git clone https://github.com/cygnusxi/CurecoinSource.git
cd CurecoinSource
qmake && make
./curecoin-qt
```

### GUI Wallet Linux Example: Install Steps on older Mint 19 / Ubuntu 18.04:
```
sudo apt install git
sudo apt-get install qt5-default qt5-qmake qtbase5-dev-tools qttools5-dev-tools libboost-dev libboost-system-dev libboost-filesystem-dev libboost-program-options-dev libboost-thread-dev libssl-dev libminiupnpc-dev libdb5.3++-dev dh-make build-essential

mkdir curecoin
cd curecoin

git clone https://github.com/cygnusxi/CurecoinSource.git
cd CurecoinSource
qmake
make

   NOTE: If it fails to compile (undefined reference to `SSLeay_version'), then you may need to do these extra steps:
     sudo apt-get update
     sudo apt-get upgrade
     sudo apt-get install libssl1.0-dev
     [Reboot]  NOTE: ensure the PC is fully updated, and 'libssl1.0-dev' is installed after the items at the beginning.
     cd curecoin/CurecoinSource
     qmake
     make

./curecoin-qt
```

### What are the commands that are accepted by the client?
From the Debug console window in the CureCoin Wallet (Help | Debug | Console), use: `help`. Common commands are:
- `listaddressgroupings` will show all of your wallet's addresses, to find any other non-zero balances in hidden wallet addresses
- `dumpprivkey <curecoinAddress>`
- `importprivkey <curecoinPrivkey> [label]`

Or, there are other commands for starting the wallet with the command line (Windows examples):
- For the command list, use: `"C:\Program Files (x86)\CureCoin\curecoin-qt.exe" -?`
- The most common one would be closing the wallet, and start it up from the command prompt once with the `-zapwallettxes` command line argument, like: `"C:\Program Files (x86)\CureCoin\curecoin-qt.exe" -zapwallettxes`, which will update and clear out any invalid transactions.
- Or, you can make a copy of a desktop shortcut and modify the Properties | Shortcut Target to be like the command above.

### Reporting Bugs or Getting Assistance

Ask questions on the [CureCoin Discord](https://discord.gg/curecoin). Other general help can be searched for on the [CureCoin forums](https://curecoin.net/forum/).

### Donations

Stars on this repo are appreciated as it helps improve the visibility of this repository. If you'd like to do more than that, you can tip CureCoins on the [CureCoin website](https://curecoin.net/).
