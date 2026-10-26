# Contributing to the Curecoin Wallet

First off, thank you for considering contributing to Curecoin! It's people like you that make Curecoin a great tool for supporting scientific research (Folding@home) through decentralized technology.

This document provides guidelines for contributing to the Curecoin wallet (Qt GUI and headless daemon).

## 1. Reporting Issues
If you find a bug or have a feature request, please open an issue on GitHub. When reporting a bug, please include:
* **Operating System & Version** (e.g., Ubuntu 26.04, Windows 11 via MSYS2)
* **Client Version** (e.g., Curecoin v2.x.x)
* **Active Libraries** (Check the Debug/Information window for your Berkeley DB, Boost, and Qt versions)
* **Steps to reproduce the behavior**
* Relevant excerpts from your `debug.log` file (please redact any sensitive balance or address information).

## 2. Development Environment & Dependencies
The Curecoin wallet relies on specific library versions to maintain cross-OS compatibility and permissive licensing. Please ensure your build environment adheres to the following:

* **Berkeley DB (BDB):** You **MUST** use BDB 5.3 (specifically 5.3.28). Do not use BDB 6.x or newer, as Oracle changed the license to AGPLv3, which is incompatible with our distribution model.
* **Boost:** We target modern Boost versions (1.89+). Note that `boost_system` is now header-only and should not be explicitly linked in makefiles or `.pro` files.
* **Qt Framework:** The GUI is built on Qt 5. 
* **Other core dependencies:** OpenSSL (1.1 or 3.x), MiniUPnPc, libevent.

### Build Instructions Reference
* **Ubuntu/Debian:** Use standard `apt` packages (`libdb++-dev`, `libboost-all-dev`, `qtbase5-dev`, etc.).
* **Windows (MSYS2/MinGW64):** You will need to manually compile BDB 5.3 from source, as the default MSYS2 package manager distributes BDB 6.x. 

## 3. Submitting Pull Requests
We gladly accept pull requests for bug fixes, UI improvements, and core feature additions.

1. **Fork the repository** and create a feature branch (`git checkout -b feature/my-new-feature`).
2. **Write clean, documented code.** If you are adding UI elements (like new QCustomPlot charts), ensure they do not introduce heavy, bloaty dependencies.
3. **Compile and test locally** on at least one environment (Linux, Windows, or macOS). Ensure `qmake` runs cleanly without major translation file errors or missing headers.
4. **Commit your changes** with descriptive commit messages.
5. **Push to your branch** (`git push origin feature/my-new-feature`).
6. **Open a Pull Request** against the `master` branch (or specific development branch, if applicable).

## 4. Code Conventions
* **C++ Standards:** We generally follow the C++11/C++14 standards. Stick to the naming conventions and indentation styles already present in the files you are modifying (primarily derived from legacy Bitcoin/Peercoin standards).
* **UI/UX Updates:** We are modernizing the Qt Wallet. If contributing to the UI, prefer lightweight integrations (e.g., using `QCustomPlot` for 2D graphing over heavier modules) to keep the wallet snappy and the binary size small.

## 5. Security Vulnerabilities
If you discover a critical security vulnerability, please **do not** open a public issue. Instead, reach out to the core development team directly via our official Discord or developer email channels so we can patch it responsibly before public disclosure.

---
Thank you for helping us build a better wallet for the research and crypto community!
