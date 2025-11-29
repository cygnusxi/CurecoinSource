# CureCoin Testing Guide - Library Updates

## Overview

This guide provides instructions for testing the library updates made to the CureCoin wallet for compatibility with modern operating systems.

---

## Changes Made

### 1. Build System Updates

**curecoin-qt.pro:**
- Removed deprecated `BOOST_ASIO_ENABLE_OLD_SERVICES` flag (handled in code with version detection)
- Added `CONFIG += c++11` for C++11 standard support
- Updated macOS minimum version from 10.5 to 10.13 (64-bit only)
- Removed 32-bit architecture support for macOS
- Commented out hardcoded Boost library suffix for Windows (modern versions don't need it)

**All Makefiles (unix, bsd, osx, mingw, raspberrypi, android):**
- Added `-std=c++11` compiler flag
- Updated Boost library comments from "1.37" to "1.70+ compatible"
- macOS: Updated minimum version to 10.13, added C++11 flag

### 2. Documentation Updates

**README.md:**
- Updated dependency installation commands for Ubuntu 22.04+ / Debian 12+
- Changed `libdb5.3++-dev` to `libdb++-dev` (uses latest available)
- Added recommended library versions section
- Improved formatting and clarity
- Added troubleshooting section for older systems

**doc/build-unix.txt:**
- Updated dependency table with modern versions
- Added C++11 compiler requirement
- Updated recommended versions (2024)
- Updated Ubuntu/Debian installation instructions
- Added note about Boost -mt suffix no longer required

**doc/readme-qt.rst:**
- Updated dependency installation for modern Ubuntu/Debian
- Added compiler requirements section
- Updated Berkeley DB warning for 5.x/6.x versions
- Added recommended library versions section

---

## Testing Checklist

### Pre-Testing Setup

1. **Backup existing wallet:**
   ```bash
   # Backup your wallet.dat and blockchain data
   cp ~/.curecoin/wallet.dat ~/wallet.dat.backup
   ```

2. **Note current version:**
   ```bash
   ./curecoin-qt --version
   # or
   ./curecoind --version
   ```

### Platform-Specific Testing

#### Ubuntu 22.04 / 24.04 Testing

```bash
# 1. Install dependencies
sudo apt-get update
sudo apt-get install qtbase5-dev qt5-qmake qtbase5-dev-tools qttools5-dev-tools \
  cmake libboost-dev libboost-system-dev libboost-filesystem-dev \
  libboost-program-options-dev libboost-thread-dev libssl-dev \
  libminiupnpc-dev libdb++-dev dh-make build-essential

# 2. Check versions
gcc --version          # Should be 7.0+
qmake --version        # Should be Qt 5.x
dpkg -l | grep libboost  # Check Boost version
dpkg -l | grep libssl    # Check OpenSSL version
dpkg -l | grep libdb     # Check Berkeley DB version

# 3. Clean build
cd CurecoinSource
make clean  # if previously built
rm -rf build/  # remove old build artifacts

# 4. Build GUI wallet
qmake
make -j$(nproc)  # parallel build

# 5. Build headless daemon
cd src
make clean
make -f makefile.unix -j$(nproc)
cd ..

# 6. Check binaries
file ./curecoin-qt
file ./src/curecoind
# Should show: ELF 64-bit LSB executable, x86-64

# 7. Test execution
./curecoin-qt --help
./src/curecoind --help
```

#### Ubuntu 20.04 Testing (Older System)

```bash
# 1. Install dependencies (may need libdb5.3)
sudo apt-get update
sudo apt-get install qtbase5-dev qt5-qmake qtbase5-dev-tools qttools5-dev-tools \
  libboost-dev libboost-system-dev libboost-filesystem-dev \
  libboost-program-options-dev libboost-thread-dev libssl-dev \
  libminiupnpc-dev libdb5.3++-dev dh-make build-essential

# Follow same build steps as Ubuntu 22.04
```

#### macOS Testing (10.13+)

```bash
# 1. Install dependencies via Homebrew
brew install boost berkeley-db qt@5 miniupnpc openssl

# 2. Set environment variables
export BOOST_INCLUDE_PATH=/usr/local/opt/boost/include
export BOOST_LIB_PATH=/usr/local/opt/boost/lib
export OPENSSL_INCLUDE_PATH=/usr/local/opt/openssl/include
export OPENSSL_LIB_PATH=/usr/local/opt/openssl/lib
export BDB_INCLUDE_PATH=/usr/local/opt/berkeley-db/include
export BDB_LIB_PATH=/usr/local/opt/berkeley-db/lib

# 3. Build
qmake
make -j$(sysctl -n hw.ncpu)

# 4. Test
./curecoin-qt.app/Contents/MacOS/curecoin-qt --help
```

#### Windows Testing (MinGW)

```bash
# 1. Install MSYS2 and dependencies
# Follow: https://www.msys2.org/

# 2. In MSYS2 terminal:
pacman -S mingw-w64-x86_64-toolchain mingw-w64-x86_64-boost \
  mingw-w64-x86_64-qt5 mingw-w64-x86_64-db mingw-w64-x86_64-openssl

# 3. Build
qmake
mingw32-make

# 4. Test
./release/curecoin-qt.exe --help
```

---

## Functional Testing

### 1. Basic Startup Tests

```bash
# Test 1: Help output
./curecoin-qt --help
./src/curecoind --help

# Test 2: Version check
./curecoin-qt --version
./src/curecoind --version

# Test 3: GUI startup (should open without errors)
./curecoin-qt

# Test 4: Daemon startup
./src/curecoind -daemon
./src/curecoind getinfo
./src/curecoind stop
```

### 2. Network Connectivity Tests

```bash
# Start daemon
./src/curecoind -daemon

# Wait 30 seconds, then check connections
sleep 30
./src/curecoind getinfo
# Should show: "connections" > 0

# Check peer info
./src/curecoind getpeerinfo

# Stop daemon
./src/curecoind stop
```

### 3. Wallet Functionality Tests

**GUI Tests:**
1. Open wallet: `./curecoin-qt`
2. Check wallet loads without errors
3. Verify blockchain syncing starts
4. Check "Receive" tab loads
5. Check "Send" tab loads
6. Check "Transactions" tab loads
7. Check "Overview" tab shows balance

**CLI Tests:**
```bash
./src/curecoind -daemon

# Test wallet commands
./src/curecoind getinfo
./src/curecoind getbalance
./src/curecoind listaddressgroupings
./src/curecoind getnewaddress
./src/curecoind validateaddress <address>

# Test blockchain commands
./src/curecoind getblockcount
./src/curecoind getblockhash 1
./src/curecoind getblock <hash>

# Test network commands
./src/curecoind getpeerinfo
./src/curecoind getnetworkinfo

./src/curecoind stop
```

### 4. RPC Server Tests

```bash
# Create curecoin.conf
mkdir -p ~/.curecoin
cat > ~/.curecoin/curecoin.conf << EOF
rpcuser=test
rpcpassword=test123
rpcport=9912
server=1
EOF

# Start daemon
./src/curecoind -daemon

# Test RPC
curl --user test:test123 --data-binary '{"jsonrpc": "1.0", "id":"test", "method": "getinfo", "params": [] }' -H 'content-type: text/plain;' http://127.0.0.1:9912/

./src/curecoind stop
```

### 5. Staking Tests (if applicable)

```bash
# Start with unlocked wallet
./curecoin-qt

# In Debug Console (Help -> Debug Window -> Console):
getinfo
getstakinginfo

# Check if staking is enabled and working
```

---

## Compiler Warning Checks

During compilation, watch for:

1. **Deprecated API warnings:**
   - Boost deprecated functions
   - OpenSSL deprecated functions
   - Qt deprecated functions

2. **C++ standard warnings:**
   - Should compile with C++11 without errors
   - No "requires C++14" or higher warnings

3. **Library version warnings:**
   - Check for any version mismatch warnings

---

## Performance Testing

### 1. Sync Speed Test

```bash
# Remove blockchain (backup first!)
rm -rf ~/.curecoin/blocks ~/.curecoin/chainstate

# Start fresh sync
time ./src/curecoind -daemon

# Monitor sync progress
watch -n 10 './src/curecoind getinfo'

# Record time to sync to current height
```

### 2. Memory Usage Test

```bash
# Start daemon
./src/curecoind -daemon

# Monitor memory usage
watch -n 5 'ps aux | grep curecoind'

# Record peak memory usage
```

---

## Known Issues & Workarounds

### Issue 1: Berkeley DB Version Mismatch

**Symptom:** Wallet fails to open with "DB version mismatch" error

**Solution:**
```bash
# Check BDB version
ldd ./curecoin-qt | grep libdb
# or
ldd ./src/curecoind | grep libdb

# Rebuild with specific BDB version
make clean
qmake BDB_LIB_SUFFIX=-5.3  # or -6.2, etc.
make
```

### Issue 2: Boost ASIO Deprecated Services

**Symptom:** Compilation warnings about BOOST_ASIO_ENABLE_OLD_SERVICES

**Solution:** Already handled in `src/curecoinrpc.h` with version detection. No action needed.

### Issue 3: OpenSSL 3.0 Compatibility

**Symptom:** Compilation errors with OpenSSL 3.0

**Solution:** May need code updates for OpenSSL 3.0 API changes. Check `src/key.cpp` and `src/crypter.cpp`.

---

## Regression Testing

Compare new build with old build:

1. **Transaction creation:** Create and send test transaction
2. **Address generation:** Generate new addresses
3. **Backup/restore:** Test wallet backup and restore
4. **Encryption:** Test wallet encryption/decryption
5. **Staking:** Verify staking still works (if applicable)

---

## Reporting Issues

If you encounter issues:

1. **Collect information:**
   ```bash
   uname -a
   gcc --version
   qmake --version
   dpkg -l | grep -E 'libboost|libssl|libdb|qt5'
   ```

2. **Capture build log:**
   ```bash
   make clean
   qmake > build.log 2>&1
   make >> build.log 2>&1
   ```

3. **Capture runtime errors:**
   ```bash
   ./curecoin-qt --debug > runtime.log 2>&1
   ```

4. **Check debug.log:**
   ```bash
   tail -n 100 ~/.curecoin/debug.log
   ```

---

## Success Criteria

✅ Compiles without errors on Ubuntu 22.04+
✅ Compiles without errors on Ubuntu 20.04
✅ Compiles without errors on macOS 10.13+
✅ Compiles without errors on Windows 10/11
✅ Wallet starts and connects to network
✅ Can create and send transactions
✅ Can generate new addresses
✅ RPC server works correctly
✅ Staking works (if applicable)
✅ No memory leaks detected
✅ Performance is comparable to previous version

---

## Next Steps After Testing

1. **If all tests pass:**
   - Tag release: `git tag v2.2.0-beta`
   - Create release notes
   - Community beta testing

2. **If issues found:**
   - Document issues
   - Create fixes
   - Re-test

3. **Future improvements:**
   - Consider Peercoin integration (Phase 2)
   - Protocol version updates
   - Security audit

