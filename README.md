# CureCoin Wallet

## Overview
If you are a newcomer, check out the [CureCoin website](https://curecoin.net/) which explains CureCoin in simple terms on the front page with a brief video.

## Joining the Network
Below you will find instructions for Windows, Linux, and Mac computers to install the CureCoin wallet.

### Folding@home (Any Platform)
Setup for earning CureCoin only using the standard Folding@home client:

1. Install the [Folding@home software](https://foldingathome.org/start-folding/).
2. Pick a Folding@home Username & get a [Passkey](https://apps.foldingathome.org/getpasskey).
3. Enter `224497` as the Folding@home Team number.
4. Register on the [Curecoin Research Core website](https://researchcore.curecoin.net/) with the **exact** same username (50 characters or less, **CaSE SensiTIVE**) used in Folding@home.
5. Complete some Folding@home Work Units, and expect your CureCoins within 24-32 hours!

---

## Standard Wallet Installation

Download the latest wallet for Windows, Mac, or Linux:
[**CureCoin GitHub Releases**](https://github.com/cygnusxi/CurecoinSource/releases)

**Important:** CureCoin v2.0 is a mandatory update. Older wallets must be updated to this version (or newer) to work correctly. If you used CureCoin in the distant past (pre-2018), you may need to resync the entire chain if you recently tried syncing with your 1.0 wallet.

### File Locations
The blockchain, `wallet.dat`, and configuration files (`curecoin.conf`) are located in the following hidden folders:

| OS | Path |
| :--- | :--- |
| **Windows** | `C:\Users\YourUser\AppData\Roaming\curecoin` |
| **Linux** | `~/.curecoin` |
| **Mac** | `~/Library/Application Support/curecoin` |

### Network & Synchronization
If the wallet does not sync immediately, please be patient. Let the wallet run for a few hours to find nodes.

**Optional Port Forwarding:**
If your wallet has 0 connections or is not syncing after a long period, you may need to ensure your network allows incoming connections.
* **Port Forward:** Forward TCP port `9911` on your router to the wallet PC IP address.
* This is generally only required if you wish to help the network by accepting incoming peer connections.

**Seed Nodes:**
If you cannot connect, you can manually add seed nodes to your `curecoin.conf` file using IPs found on the [Network 'Node List'](https://chainz.cryptoid.info/cure/#!network):

```ini
addnode=111.111.111.111
addnode=222.222.222.222
```

**Network Stats:**
You can find nodes to use for `addnode` if need be on the network tab of the block explorer.

* **Research Rates:** [Team Stats Page](https://folding.extremeoverclocking.com/team_summary.php?s=&t=224497)
* **Blockchain Status:** [CureCoin Block Explorer](https://chainz.cryptoid.info/cure/)

---

## Linux Installation From Source

These instructions are for compiling the wallet yourself.

* **GUI Client (`curecoin-qt`):** Recommended for most users.
* **Headless Client (`curecoind`):** Command-line only, best for servers/expert users.

### 1. Prepare Environment
Install Git and clone the repository:
```bash
sudo apt install git
git clone [https://github.com/cygnusxi/CurecoinSource.git](https://github.com/cygnusxi/CurecoinSource.git)
```

### 2. Install Dependencies
**Target Systems:** Ubuntu 22.04+, Mint 21.1+, Debian 12+

```bash
sudo apt-get install qtbase5-dev qt5-qmake qtbase5-dev-tools qttools5-dev-tools \
  cmake libboost-dev libboost-system-dev libboost-filesystem-dev \
  libboost-program-options-dev libboost-thread-dev libssl-dev \
  libminiupnpc-dev libdb++-dev dh-make build-essential
```
*(Note: For older systems like Ubuntu 18.04, replace `libdb++-dev` with `libdb5.3++-dev`)*

### 3. Compile

#### Option A: GUI Wallet (curecoin-qt)
```bash
cd CurecoinSource
qmake && make
./curecoin-qt
```

#### Option B: Headless Wallet (curecoind)
```bash
cd CurecoinSource/src
make -f makefile.unix
sudo make install
```
* Run with: `./curecoind`
* Check status: `./curecoind getinfo`
* List commands: `./curecoind help`

### Recommended Build Versions
* **Qt:** 5.15+ (Qt 6.x also supported)
* **Boost:** 1.70+
* **OpenSSL:** 1.1.1+ or 3.0+
* **Berkeley DB:** 5.3+ or 6.x
* **Compiler:** C++11 compatible (GCC 7+, Clang 5+)

---

## Wallet Commands & Troubleshooting

### Debug Console
From the GUI Wallet, navigate to **Help | Debug | Console**. Common commands:

* `help`: List all commands.
* `listaddressgroupings`: Show all wallet addresses (helps find balances in change addresses).
* `dumpprivkey <curecoinAddress>`: Reveal the private key for a specific address. (**Warning:** Never share your private key with anyone).
* `importprivkey <curecoinPrivkey> [label]`: Import a private key.

### Command Line Arguments (Windows Example)
You can run the wallet with specific flags by modifying your desktop shortcut target.

**Rescan/Repair:**
`"C:\Program Files (x86)\CureCoin\curecoin-qt.exe" -zapwallettxes`
> Use this if your balance is incorrect or transactions are stuck. It cleans invalid transactions and rescans.

**List Arguments:**
`"C:\Program Files (x86)\CureCoin\curecoin-qt.exe" -?`

---

## Support & Contributing

* **Discord:** [CureCoin Discord](https://discord.gg/curecoin)
* **Forums:** [CureCoin Forums](https://curecoin.net/forum/)

Stars on this repo are appreciated as they help improve visibility. If you'd like to support further, you can tip CureCoins via the [CureCoin website](https://curecoin.net/).