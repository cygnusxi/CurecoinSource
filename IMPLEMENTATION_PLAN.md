# CureCoin Upgrade Implementation Plan

## Quick Start: Library Updates

This document provides step-by-step instructions for updating CureCoin dependencies.

---

## Step 1: Update Qt Project File (curecoin-qt.pro)

### Changes Needed:

1. **Remove deprecated Boost flag** (line 5):
   - Remove: `BOOST_ASIO_ENABLE_OLD_SERVICES`
   - Note: Already handled in `src/curecoinrpc.h` with version detection

2. **Add C++11/17 support**:
   - Add: `CONFIG += c++11` or `CONFIG += c++17`

3. **Update macOS build targets** (line 37):
   - Change from: `-mmacosx-version-min=10.5 -arch i386`
   - To: `-mmacosx-version-min=10.13` (or higher)
   - Remove: `-arch i386` (32-bit deprecated)

4. **Update Boost library suffix defaults** (line 313):
   - Change from: `-mgw44-mt-s-1_50`
   - To: Modern Boost versions don't need this suffix format

---

## Step 2: Update Makefiles

### Files to Update:
- `src/makefile.unix`
- `src/makefile.bsd`
- `src/makefile.mingw`
- `src/makefile.osx`
- `src/makefile.raspberrypi`

### Changes Needed:

1. **Add C++11/17 flag**:
   ```makefile
   xCXXFLAGS=-O2 -msse2 -pthread -std=c++11 -Wall -Wextra ...
   ```

2. **Update Boost comment** (line 28):
   - Change from: `# for boost 1.37, add -mt to the boost libraries`
   - To: `# Boost libraries (1.70+ compatible)`

3. **Update library paths**:
   - Ensure BDB paths point to version 6.x if available
   - Update OpenSSL paths for version 3.0+

---

## Step 3: Update README.md

### Update Dependency Lists:

**For Ubuntu 22.04+ / Debian 12+:**
```bash
sudo apt-get install qtbase5-dev qt5-qmake qtbase5-dev-tools qttools5-dev-tools \
  cmake libboost-dev libboost-system-dev libboost-filesystem-dev \
  libboost-program-options-dev libboost-thread-dev libssl-dev \
  libminiupnpc-dev libdb++-dev dh-make build-essential
```

**Note:** `libdb5.3++-dev` → `libdb++-dev` (uses latest available)

---

## Step 4: Test Compilation

### Test on Each Platform:

1. **Linux (Ubuntu 22.04+)**:
   ```bash
   cd CurecoinSource
   qmake
   make
   ```

2. **Windows (MinGW/MSVC)**:
   - Update Windows build instructions
   - Test with modern MinGW-w64

3. **macOS**:
   - Test with Xcode 13+
   - Ensure 64-bit only build

---

## Step 5: Code Compatibility Fixes

### Files That May Need Updates:

1. **src/util.cpp**:
   - Check Boost filesystem API usage
   - Update if using deprecated APIs

2. **src/db.cpp**:
   - Check Berkeley DB API compatibility
   - May need updates for BDB 6.x

3. **src/curecoinrpc.cpp**:
   - Already has Boost 1.70+ compatibility
   - Verify OpenSSL 3.0 compatibility

---

## Testing Checklist

- [ ] Compiles on Ubuntu 22.04
- [ ] Compiles on Ubuntu 24.04
- [ ] Compiles on Windows 10/11
- [ ] Compiles on macOS 13+
- [ ] Wallet starts and connects to network
- [ ] Can create transactions
- [ ] Can stake coins (if applicable)
- [ ] RPC server works
- [ ] Blockchain syncs correctly

---

## Rollback Plan

If issues occur:
1. Keep old build files in git history
2. Tag current working version: `git tag v2.0.0.1-stable`
3. Create branch for updates: `git checkout -b upgrade-dependencies`
4. Test thoroughly before merging

---

## Next Steps After Library Updates

Once libraries are updated:
1. Run security audit
2. Update protocol version (if needed)
3. Begin Peercoin integration planning
4. Community testing and feedback

