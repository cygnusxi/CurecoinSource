CHANGELOG.md

2.2.0.1 Technical Update: Curecoin-Qt Migration to Modern Toolchain (MSYS2/MinGW64)
Objective: Compile legacy C++11 codebase on modern Windows using GCC 15+, Qt 5.15+, and Boost 1.85+.

1. Source Code Refactoring (C++ Modernization)
Extensive patching was required to resolve deprecations in the Boost library and stricter type-checking in modern GCC compilers.

Boost.Filesystem Updates (src/util.cpp, src/walletdb.cpp, src/init.cpp)

Replaced deprecated .is_complete() with .is_absolute().

Removed obsolete header #include <boost/filesystem/convenience.hpp>.

Updated copy_file logic: Replaced copy_option::overwrite_if_exists with the modern plural copy_options::overwrite_existing.

Boost.ASIO / Networking (src/curecoinrpc.cpp)

Renaming: Replaced all instances of io_service with io_context.

Renaming: Replaced socket_base::max_connections with socket_base::max_listen_connections.

Resolver Logic: Rewrote tcp::resolver implementation to use results_type and iterators instead of the deprecated query method.

IP Handling: Replaced deprecated .is_v4_compatible() and .to_ulong() checks with modern make_address_v4() and .to_uint() logic.

Berkeley DB Compatibility (src/db.cpp)

Resolved ambiguous constructor call for DbEnv(0) by explicitly casting to DbEnv((u_int32_t)0).

Safety Fixes (src/util.cpp)

Fixed an integer overflow warning in fseek by casting sizeof(pch) to (long).

2. Project Configuration Changes (curecoin-qt.pro)
Modifications to the QMake project file to handle modern libraries and linker flags.

Boost Configuration:

Defined BOOST_BIND_GLOBAL_PLACEHOLDERS to silence bind warnings.

Defined BOOST_ASIO_ENABLE_OLD_SERVICES to maintain compatibility with legacy RPC code structures.

Removed libboost_system from linker flags (library is now header-only in recent versions).

Added win32:BOOST_LIB_SUFFIX = -mt to locate the correct Multithreaded static libraries on MinGW64.

Static/Dynamic Linking Mix:

Implemented -Wl,-Bstatic and -Wl,-Bdynamic wrapping to statically link Boost libraries while keeping system libraries dynamic.

2.2.0.2 Fixed an integer overflow warning in fseek by casting sizeof(pch) to (long). (this was a potential crash issue with the debug log)

2.2.0.3 Removed the irc.h and irc.cpp files from the project. (this was a potential security issue)

2.2.0.4 Tweaking DNS function timings tp speed up peer discovery

2.2.0.5 strlcpy fix, compiling fixes for windows and linux

2.2.0.6 Critical RPC function syntax fix in curecoinrpc.cpp. Fixed broken ClientAllowed function that was causing HTTP 403 Forbidden errors for legitimate RPC connections.

2.2.0.7 Backward compatible Boost fixes and code cleanup. Improved Boost library compatibility and cleaned up deprecated code patterns.