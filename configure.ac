#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.61)
#
# The 0MQ version number is extracted from include/zmq.h using
# the version.sh script. Hence, it should be updated there.
# The version in git should reflect the *next* version planned.
#
AC_INIT([zeromq],[m4_esyscmd([./version.sh])],[zeromq-dev@lists.zeromq.org])

AC_CONFIG_AUX_DIR(config)
AC_CONFIG_MACRO_DIR(config)
AC_CONFIG_HEADERS([src/platform.hpp])
AM_INIT_AUTOMAKE(foreign subdir-objects tar-ustar dist-zip)

m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

# This lets us use PACKAGE_VERSION in Makefiles
AC_SUBST(PACKAGE_VERSION)

# Libtool -version-info (ABI version)
#
# Don't change this unless you know exactly what you're doing and have read and
# understand:
# http://www.gnu.org/software/libtool/manual/html_node/Updating-version-info.html
#
# Changes:
#
# ZeroMQ versions prior to 2.1.0 use 0:0:0 (undefined)
# ZeroMQ versions 2.1.x: 1:0:0 (ABI version 1)
# ZeroMQ version 3.0: 2:0:0 (ABI version 2)
# ZeroMQ version 3.1: 3:0:0 (ABI version 3)
# ZeroMQ version 4.0: 4:0:0 (ABI version 4)
# ZeroMQ version 4.1: 5:0:0 (ABI version 5)
#
# libzmq -version-info current:revision:age
LTVER="5:0:0"
AC_SUBST(LTVER)

# Take a copy of original flags
ZMQ_ORIG_CFLAGS="${CFLAGS:-none}"
ZMQ_ORIG_CPPFLAGS="${CPPFLAGS:-none}"
ZMQ_ORIG_CXXFLAGS="${CXXFLAGS:-none}"

# Checks for programs.
AC_PROG_CC
AC_PROG_CC_C99
AC_PROG_CXX
AM_PROG_CC_C_O
AC_PROG_SED
AC_PROG_AWK
PKG_PROG_PKG_CONFIG
m4_pattern_forbid([^PKG_[A-Z_]+$], [missing some pkg-config macros (pkg-config package)])

# Libtool configuration for different targets. See acinclude.m4
AC_ARG_VAR([XMLTO], [Path to xmlto command])
AC_PATH_PROG([XMLTO], [xmlto])
AC_ARG_VAR([ASCIIDOC], [Path to asciidoc command])
AC_PATH_PROG([ASCIIDOC], [asciidoc])
LIBZMQ_CONFIG_LIBTOOL
AC_LIBTOOL_WIN32_DLL
AC_PROG_LIBTOOL

# Check whether to build a with debug symbols
LIBZMQ_CHECK_ENABLE_DEBUG

# Check wheter to enable code coverage
LIBZMQ_WITH_GCOV



AC_MSG_CHECKING([if TIPC is available and supports nonblocking connect])

AC_RUN_IFELSE(
    [AC_LANG_PROGRAM([[
            #include <stdlib.h>
            #include <string.h>
            #include <fcntl.h>
            #include <errno.h>
            #include <sys/socket.h>
            #include <linux/tipc.h>
        ]],[[
            struct sockaddr_tipc topsrv;
            int sd = socket(AF_TIPC, SOCK_SEQPACKET, 0);
            if (sd == -EAFNOSUPPORT) {
                return 1;
            }
            memset(&topsrv, 0, sizeof(topsrv));
            topsrv.family = AF_TIPC;
            topsrv.addrtype = TIPC_ADDR_NAME;
            topsrv.addr.name.name.type = TIPC_TOP_SRV;
            topsrv.addr.name.name.instance = TIPC_TOP_SRV;
            fcntl(sd, F_SETFL, O_NONBLOCK);
            if (connect(sd, (struct sockaddr *)&topsrv, sizeof(topsrv)) != 0) {
                if (errno != EINPROGRESS)
                    return -1;
            }
        ]])
    ],
    [libzmq_tipc_support=yes],
    [libzmq_tipc_support=no],
    [libzmq_tipc_support=no])

AC_MSG_RESULT([$libzmq_tipc_support])


AC_ARG_WITH([relaxed],
    [AS_HELP_STRING([--with-relaxed],
        [Switch off pedantic compiler])],
    [zmq_relaxed="yes"],
    [])

if test "x$zmq_relaxed" = "xyes"; then
    libzmq_pedantic="no"
else
    libzmq_pedantic="yes"
fi

AC_ARG_WITH([militant],
    [AS_HELP_STRING([--with-militant],
        [Enable militant API assertions])],
    [zmq_militant="yes"],
    [])

if test "x$zmq_militant" = "xyes"; then
    AC_DEFINE(ZMQ_ACT_MILITANT, 1, [Enable militant API assertions])
fi


# By default compiling with -Werror except OSX.
libzmq_werror="yes"

# By default use DSO visibility
libzmq_dso_visibility="yes"

# Platform specific checks
libzmq_on_mingw32="no"
libzmq_on_cygwin="no"
libzmq_on_android="no"
libzmq_on_linux="no"

# Set some default features required by 0MQ code.
CPPFLAGS="-D_REENTRANT -D_THREAD_SAFE $CPPFLAGS"

# For host type checks
AC_CANONICAL_HOST

# OS-specific tests
case "${host_os}" in
    *linux*)
        # Define on Linux to enable all library features. Define if using a gnu compiler
        if test "x$GXX" = "xyes"; then
            CPPFLAGS="-D_GNU_SOURCE $CPPFLAGS"
        fi
        AC_DEFINE(ZMQ_HAVE_LINUX, 1, [Have Linux OS])
        libzmq_on_linux="yes"

        if test "x$libzmq_tipc_support" = "xyes"; then
            AC_DEFINE(ZMQ_HAVE_TIPC, 1, [Have TIPC support])
        fi
        case "${host_os}" in
            *android*)
                AC_DEFINE(ZMQ_HAVE_ANDROID, 1, [Have Android OS])
                libzmq_on_android="yes"
            ;;
        esac
        ;;
    *solaris*)
        # Define on Solaris to enable all library features
        CPPFLAGS="-D_PTHREADS $CPPFLAGS"
        AC_DEFINE(ZMQ_HAVE_SOLARIS, 1, [Have Solaris OS])
        AC_CHECK_LIB(socket, socket)
        AC_CHECK_LIB(nsl, gethostbyname)
        AC_MSG_CHECKING([whether atomic operations can be used])
        AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
            [[#include <atomic.h>]],
            [[uint32_t value;
              atomic_cas_32 (&value, 0, 0);
              return 0;]])],
            [solaris_has_atomic=yes],
            [solaris_has_atomic=no])
        AC_MSG_RESULT([$solaris_has_atomic])
        # Solaris 8 does not have atomic operations exported to user space.
        if test "x$solaris_has_atomic" = "xno"; then
            AC_DEFINE(ZMQ_FORCE_MUTEXES, 1, [Force to use mutexes])
        fi
        # ssp library is required for libsodium on Solaris-like systems
        LDFLAGS="-lssp $LDFLAGS"
        CPPFLAGS="$CPPFLAGS -Wno-long-long"
        ;;
    *freebsd*)
        # Define on FreeBSD to enable all library features
        CPPFLAGS="-D__BSD_VISIBLE $CPPFLAGS"
        AC_DEFINE(ZMQ_HAVE_FREEBSD, 1, [Have FreeBSD OS])
        ;;
    *darwin*)
        # Define on Darwin to enable all library features
        CPPFLAGS="-D_DARWIN_C_SOURCE $CPPFLAGS"
        libzmq_pedantic="no"
        libzmq_werror="no"
        AC_DEFINE(ZMQ_HAVE_OSX, 1, [Have DarwinOSX OS])
        AC_LANG_PUSH([C++])
        LIBZMQ_CHECK_LANG_FLAG_PREPEND([-Wno-uninitialized])
        AC_LANG_POP([C++])
        ;;
    *netbsd*)
        # Define on NetBSD to enable all library features
        CPPFLAGS="-D_NETBSD_SOURCE $CPPFLAGS"
        AC_DEFINE(ZMQ_HAVE_NETBSD, 1, [Have NetBSD OS])
        # NetBSD 5.0 and newer provides atomic operations but we can
        # only use these on systems where PR #42842 has been fixed so
        # we must try and link a test program using C++.
        libzmq_netbsd_has_atomic=no
        AC_MSG_CHECKING([whether atomic operations can be used])
        AC_LANG_PUSH([C++])
        AC_LINK_IFELSE([AC_LANG_PROGRAM(
            [[#include <atomic.h>]],
            [[uint32_t value;
              atomic_cas_32 (&value, 0, 0);
              return 0;]])],
            [libzmq_netbsd_has_atomic=yes],
            [libzmq_netbsd_has_atomic=no])
        AC_LANG_POP([C++])
        AC_MSG_RESULT([$libzmq_netbsd_has_atomic])
        if test "x$libzmq_netbsd_has_atomic" = "xno"; then
            AC_DEFINE(ZMQ_FORCE_MUTEXES, 1, [Force to use mutexes])
        fi
        ;;
    *openbsd*|*bitrig*)
        # Define on OpenBSD to enable all library features
        CPPFLAGS="-D_BSD_SOURCE $CPPFLAGS"
        AC_DEFINE(ZMQ_HAVE_OPENBSD, 1, [Have OpenBSD OS])
        ;;
    *nto-qnx*)
        libzmq_pedantic="no"
        AC_DEFINE(ZMQ_HAVE_QNXNTO, 1, [Have QNX Neutrino OS])
        AC_CHECK_LIB(socket, socket)
        ;;
    *aix*)
        AC_DEFINE(ZMQ_HAVE_AIX, 1, [Have AIX OS])
        ;;
    *hpux*)
        # Define on HP-UX to enable all library features
        CPPFLAGS="-D_POSIX_C_SOURCE=200112L $CPPFLAGS"
        AC_DEFINE(ZMQ_HAVE_HPUX, 1, [Have HPUX OS])
        LIBZMQ_CHECK_LANG_FLAG_PREPEND([-Ae])
        AC_CHECK_FUNCS(gethrtime)
        ;;
    *mingw*)
        AC_DEFINE(ZMQ_HAVE_WINDOWS, 1, [Have Windows OS])
        AC_DEFINE(ZMQ_HAVE_MINGW32, 1, [Have MinGW32])
        AC_CHECK_HEADERS(windows.h)
        AC_CHECK_LIB(ws2_32, main, ,
            [AC_MSG_ERROR([cannot link with ws2_32.dll.])])
        AC_CHECK_LIB(rpcrt4, main, ,
            [AC_MSG_ERROR([cannot link with rpcrt4.dll.])])
        AC_CHECK_LIB(iphlpapi, main, ,
            [AC_MSG_ERROR([cannot link with iphlpapi.dll.])])
        # mingw32 defines __int64_t as long long
        AC_LANG_PUSH([C++])
        LIBZMQ_CHECK_LANG_FLAG_PREPEND([-Wno-long-long])
        AC_LANG_POP([C++])
        libzmq_on_mingw32="yes"
        libzmq_dso_visibility="no"

        if test "x$enable_static" = "xyes"; then
            AC_MSG_ERROR([Building static libraries is not supported under MinGW32])
        fi
        
	# Set FD_SETSIZE to 1024
	CPPFLAGS=" -DFD_SETSIZE=1024 $CPPFLAGS"
        ;;
    *cygwin*)
        # Define on Cygwin to enable all library features
        CPPFLAGS="-D_GNU_SOURCE $CPPFLAGS"
        AC_DEFINE(ZMQ_HAVE_CYGWIN, 1, [Have Cygwin])
        libzmq_on_cygwin="yes"
        libzmq_dso_visibility="no"
        if test "x$enable_static" = "xyes"; then
            AC_MSG_ERROR([Building static libraries is not supported under Cygwin])
        fi
        ;;
    *)
        AC_MSG_ERROR([unsupported system: ${host_os}.])
        ;;
esac

# Checks for libraries
AC_CHECK_LIB([pthread], [pthread_create])
AC_CHECK_LIB([rt], [clock_gettime])

#
# Check if the compiler supports -fvisibility=hidden flag. MinGW32 uses __declspec
#
if test "x$libzmq_dso_visibility" = "xyes"; then
    AC_LANG_PUSH([C++])
    LIBZMQ_CHECK_LANG_VISIBILITY([LIBZMQ_EXTRA_CXXFLAGS="$libzmq_cv_[]_AC_LANG_ABBREV[]_visibility_flag ${LIBZMQ_EXTRA_CXXFLAGS}"])
    AC_LANG_POP([C++])
fi

# CPU-specific optimizations
case "${host_cpu}" in
    *sparc*)
        AC_LANG_PUSH([C++])
        LIBZMQ_CHECK_LANG_FLAG_PREPEND([-mcpu=v9])
        AC_LANG_POP([C++])
    ;;
    *)
    ;;
esac

# Check whether to build docs / install man pages
LIBZMQ_CHECK_DOC_BUILD

# Check polling system
LIBZMQ_CHECK_POLLER([CPPFLAGS="${CPPFLAGS} -D${libzmq_cv_poller_flag}"],
    [AC_MSG_ERROR([Unable to continue without polling system])])

# Checks for header files.
AC_HEADER_STDC
AC_CHECK_HEADERS(\
    errno.h \
    time.h \
    unistd.h \
    limits.h \
    stddef.h \
    stdlib.h \
    string.h \
    arpa/inet.h \
    netinet/tcp.h \
    netinet/in.h \
    sys/socket.h \
    sys/time.h)

# Check if we have ifaddrs.h header file.
AC_CHECK_HEADERS(ifaddrs.h, [AC_DEFINE(ZMQ_HAVE_IFADDRS, 1, [Have ifaddrs.h header.])])

# Check if we have sys/uio.h header file.
AC_CHECK_HEADERS(sys/uio.h, [AC_DEFINE(ZMQ_HAVE_UIO, 1, [Have uio.h header.])])

# Force not to use eventfd
AC_ARG_ENABLE([eventfd],
    [AS_HELP_STRING([--disable-eventfd], [disable eventfd [default=no]])],
    [zmq_enable_eventfd=$enableval],
    [zmq_enable_eventfd=yes])

if test "x$zmq_enable_eventfd" = "xyes"; then
    # Check if we have eventfd.h header file.
    AC_CHECK_HEADERS(sys/eventfd.h,
        [AC_DEFINE(ZMQ_HAVE_EVENTFD, 1, [Have eventfd extension.])])
fi

# Conditionally build performance measurement tools
AC_ARG_ENABLE([perf],
    [AS_HELP_STRING([--enable-perf], [Build performance measurement tools [default=yes].])],
    [zmq_enable_perf=$enableval],
    [zmq_enable_perf=yes])

AM_CONDITIONAL(ENABLE_PERF, test "x$zmq_enable_perf" = "xyes")

# Conditionally build curve key generation tool
AC_ARG_ENABLE([curve-keygen],
    [AS_HELP_STRING([--enable-curve-keygen], [Build curve key-generation tool [default=yes].])],
    [zmq_enable_curve_keygen=$enableval],
    [zmq_enable_curve_keygen=yes])

AM_CONDITIONAL(ENABLE_CURVE_KEYGEN, test "x$zmq_enable_curve_keygen" = "xyes")

# Use c++ in subsequent tests
AC_LANG_PUSH(C++)

AC_CHECK_DECLS([SO_PEERCRED],
    [AC_DEFINE(ZMQ_HAVE_SO_PEERCRED, 1, [Have SO_PEERCRED socket option])],
    [],
    [#include <sys/socket.h>])

AC_CHECK_DECLS([LOCAL_PEERCRED],
    [AC_DEFINE(ZMQ_HAVE_LOCAL_PEERCRED, 1, [Have LOCAL_PEERCRED socket option])],
    [],
    [#include <sys/socket.h>])

AM_CONDITIONAL(HAVE_IPC_PEERCRED, test "x$ac_cv_have_decl_SO_PEERCRED" = "xyes" || test "x$ac_cv_have_decl_LOCAL_PEERCRED" = "xyes")

AC_HEADER_STDBOOL
AC_C_CONST
AC_C_INLINE

# Checks for typedefs, structures, and compiler characteristics.
if test "x$libzmq_cv_[]_AC_LANG_ABBREV[]_intel_compiler" = "xyes"; then
    dnl 279: controlling expression is constant
    dnl Fixes build with ICC 12.x
    LIBZMQ_CHECK_WITH_FLAG([-wd279], [AC_TYPE_SIZE_T])
    LIBZMQ_CHECK_WITH_FLAG([-wd279], [AC_TYPE_SSIZE_T])
else
    AC_TYPE_SIZE_T
    AC_TYPE_SSIZE_T
fi

AC_HEADER_TIME
AC_TYPE_UINT32_T
AC_C_VOLATILE

# build using libgssapi_krb5
AC_ARG_WITH([libgssapi_krb5], [AS_HELP_STRING([--with-libgssapi_krb5],
    [require libzmq build with libgssapi_krb5 [default=no]])],
    [require_libgssapi_krb5_ext=$withval],
    [require_libgssapi_krb5_ext=no])

# conditionally require libgssapi_krb5
if test "x$require_libgssapi_krb5_ext" != "xno"; then
    AC_CHECK_LIB([gssapi_krb5], [gss_init_sec_context],,
        AC_MSG_ERROR(libgssapi_krb5 is needed for GSSAPI security))
fi

# build using libsodium
have_sodium_library="no"

AC_ARG_WITH([libsodium], [AS_HELP_STRING([--with-libsodium],
    [require libzmq build with libsodium. Requires pkg-config [default=no]])],
    [require_libsodium_ext=$withval],
    [require_libsodium_ext=yes])

# conditionally require libsodium package
if test "x$require_libsodium_ext" != "xno"; then
    PKG_CHECK_MODULES([sodium], [libsodium], [have_sodium_library="yes"])
fi

if test "x$have_sodium_library" != "xno"; then
    AC_DEFINE(HAVE_LIBSODIUM, 1, [The libsodium library is to be used.])
fi

AM_CONDITIONAL(HAVE_SODIUM, test "x$have_sodium_library" != "xno")

# build using pgm
have_pgm_library="no"

AC_ARG_WITH([pgm], [AS_HELP_STRING([--with-pgm],
    [build libzmq with PGM extension. Requires pkg-config [default=no]])],
    [with_pgm_ext=$withval],
    [with_pgm_ext=no])

# conditionally require pgm package
if test "x$with_pgm_ext" != "xno"; then
    PKG_CHECK_MODULES([pgm], [openpgm-5.2 >= 5.2], [ have_pgm_library="yes" ],
        [PKG_CHECK_MODULES([pgm], [openpgm-5.1 >= 5.1],
            [ have_pgm_library="yes" ])])
fi

if test "x$have_pgm_library" = "xyes"; then
    AC_DEFINE(ZMQ_HAVE_OPENPGM, [1], [Have OpenPGM extension])
fi

AM_CONDITIONAL(HAVE_PGM, test "x$have_pgm_library" = "xyes")


# This uses "--with-norm" to point to the "norm" directory
# for "norm/include" and "norm/lib"
#(if "--with-norm=yes" is given, then assume installed on system)
AC_ARG_WITH([norm],
    [AS_HELP_STRING([--with-norm],
        [build libzmq with NORM protocol extension, optionally specifying norm path [default=no]])],
    [with_norm_ext=$withval],
    [with_norm_ext=no])



AC_MSG_CHECKING("with_norm_ext = ${with_norm_ext}")

if test "x$with_norm_ext" != "xno"; then
    AC_DEFINE(ZMQ_HAVE_NORM, 1, [Have NORM protocol extension])

    if test "x$wwith_norm_ext" != "xyes"; then
        norm_path="${with_norm_ext}"
        LIBZMQ_EXTRA_CXXFLAGS="-I${norm_path}/include ${LIBZMQ_EXTRA_CXXFLAGS}"
        LIBZMQ_EXTRA_LDFLAGS="-I${norm_path}/include ${LIBZMQ_EXTRA_LDFLAGS}"
    fi

    LIBS="-lnorm $LIBS"
fi

# Set -Wall, -Werror and -pedantic
AC_LANG_PUSH([C++])

# Check how to enable -Wall
LIBZMQ_LANG_WALL([CPPFLAGS="$libzmq_cv_[]_AC_LANG_ABBREV[]_wall_flag $CPPFLAGS"])

if test "x$libzmq_werror" = "xyes" -a "x$libzmq_cv_[]_AC_LANG_ABBREV[]_sun_studio_compiler" != "xyes"; then
    LIBZMQ_LANG_WERROR([CPPFLAGS="$libzmq_cv_[]_AC_LANG_ABBREV[]_werror_flag $CPPFLAGS"])
fi

if test "x$libzmq_pedantic" = "xyes"; then
    LIBZMQ_LANG_STRICT([CPPFLAGS="$libzmq_cv_[]_AC_LANG_ABBREV[]_strict_flag $CPPFLAGS"])
fi
AC_LANG_POP([C++])

AM_CONDITIONAL(BUILD_TIPC, test "x$libzmq_tipc_support" = "xyes")
AM_CONDITIONAL(ON_MINGW, test "x$libzmq_on_mingw32" = "xyes")
AM_CONDITIONAL(ON_CYGWIN, test "x$libzmq_on_cygwin" = "xyes")
AM_CONDITIONAL(ON_ANDROID, test "x$libzmq_on_android" = "xyes")
AM_CONDITIONAL(ON_LINUX, test "x$libzmq_on_linux" = "xyes")

# Check for __atomic_Xxx compiler intrinsics
AC_LANG_PUSH([C++])
LIBZMQ_CHECK_ATOMIC_INTRINSICS([
    AC_DEFINE([ZMQ_HAVE_ATOMIC_INTRINSICS],
              [1],
              [Whether compiler has __atomic_Xxx intrinsics.])
    ])
AC_LANG_POP([C++])

# Checks for library functions.
AC_TYPE_SIGNAL
AC_CHECK_FUNCS(perror gettimeofday clock_gettime memset socket getifaddrs freeifaddrs fork posix_memalign)
AC_CHECK_HEADERS([alloca.h])

LIBZMQ_CHECK_SOCK_CLOEXEC([
    AC_DEFINE([ZMQ_HAVE_SOCK_CLOEXEC],
        [1],
        [Whether SOCK_CLOEXEC is defined and functioning.])
    ])

# TCP keep-alives Checks.
LIBZMQ_CHECK_SO_KEEPALIVE([
    AC_DEFINE([ZMQ_HAVE_SO_KEEPALIVE],
        [1],
        [Whether SO_KEEPALIVE is supported.])
    ])

LIBZMQ_CHECK_TCP_KEEPCNT([
    AC_DEFINE([ZMQ_HAVE_TCP_KEEPCNT],
        [1],
        [Whether TCP_KEEPCNT is supported.])
    ])

LIBZMQ_CHECK_TCP_KEEPIDLE([
    AC_DEFINE([ZMQ_HAVE_TCP_KEEPIDLE],
        [1],
        [Whether TCP_KEEPIDLE is supported.])
    ])

LIBZMQ_CHECK_TCP_KEEPINTVL([
    AC_DEFINE([ZMQ_HAVE_TCP_KEEPINTVL],
        [1],
        [Whether TCP_KEEPINTVL is supported.])
    ])

LIBZMQ_CHECK_TCP_KEEPALIVE([
    AC_DEFINE([ZMQ_HAVE_TCP_KEEPALIVE],
        [1],
        [Whether TCP_KEEPALIVE is supported.])
    ])

AM_CONDITIONAL(HAVE_FORK, test "x$ac_cv_func_fork" = "xyes")

# Subst LIBZMQ_EXTRA_CFLAGS & CXXFLAGS & LDFLAGS
AC_SUBST(LIBZMQ_EXTRA_CFLAGS)
AC_SUBST(LIBZMQ_EXTRA_CXXFLAGS)
AC_SUBST(LIBZMQ_EXTRA_LDFLAGS)

# set pkgconfigdir, allow override
AC_ARG_WITH([pkgconfigdir],
    AS_HELP_STRING([--with-pkgconfigdir=PATH],
        [Path to the pkgconfig directory [[LIBDIR/pkgconfig]]]),
    [pkgconfigdir="$withval"],
    [pkgconfigdir='${libdir}/pkgconfig'])
AC_SUBST([pkgconfigdir])

AC_CONFIG_FILES([ \
    Makefile \
    src/libzmq.pc \
    doc/Makefile \
    builds/Makefile \
    builds/msvc/Makefile \
    builds/redhat/zeromq.spec])

AC_OUTPUT
