#!/bin/bash
#
# Script for acquiring and building OS X dependencies for 0 A.D.
#
# The script checks whether a source tarball exists for each
# dependency, if not it will download the correct version from
# the project's website, then it removes previous build files,
# extracts the tarball, configures and builds the lib. The script
# should die on any errors to ease troubleshooting.
#
# make install is used to copy the compiled libs to each specific
# directory and also the config tools (e.g. sdl-config). Because
# of this, OS X developers must run this script at least once,
# to configure the correct lib directories. It must be run again
# if the libraries are moved.
#
# Building against an SDK is an option, though not required,
# as not all build environments contain the Developer SDKs
# (Xcode does, but the Command Line Tools package does not)
#
# --------------------------------------------------------------
# Bundled with the game:
# * SpiderMonkey 38
# * NVTT
# * FCollada
# --------------------------------------------------------------
# Provided by OS X:
# * OpenAL
# * OpenGL
# --------------------------------------------------------------
# Provided by Homebrew:
BREWED_LIBRARIES=("lzlib" "curl" "libiconv" "libxml2" "sdl2" "boost" "wxwidgets"
"libpng" "libogg" "libvorbis" "nspr" "icu4c" "enet" "miniupnpc" "libsodium"
"fftw")
# --------------------------------------------------------------
# Downloaded and compiled by this script:
# * gloox (because brew's version depends on openssl)
GLOOX_VERSION="gloox-1.0.20"
# --------------------------------------------------------------

# Force build architecture, as sometimes environment is broken.
# For a universal fat binary, the approach would be to build every
# dependency with both archs and combine them with lipo, then do the
# same thing with the game itself.
# Choices are "x86_64" or  "i386" (ppc and ppc64 not supported)
ARCH=${ARCH:="x86_64"}

# Define compiler as "clang", this is all Mavericks supports.
# gcc symlinks may still exist, but they are simply clang with
# slightly different config, which confuses build scripts.
# llvm-gcc and gcc 4.2 are no longer supported by SpiderMonkey.
export CC=${CC:="clang"} CXX=${CXX:="clang++"}
export MIN_OSX_VERSION=${MIN_OSX_VERSION:="10.9"}

# The various libs offer inconsistent configure options, some allow
# setting sysroot and OS X-specific options, others don't. Adding to
# the confusion, Apple moved /Developer/SDKs into the Xcode app bundle
# so the path can't be guessed by clever build tools (like Boost.Build).
# Sometimes configure gets it wrong anyway, especially on cross compiles.
# This is why we prefer using (OBJ)CFLAGS, (OBJ)CXXFLAGS, and LDFLAGS.

# Check if SYSROOT is set and not empty
if [[ $SYSROOT && ${SYSROOT-_} ]]; then
  C_FLAGS="-isysroot $SYSROOT"
  LDFLAGS="$LDFLAGS -Wl,-syslibroot,$SYSROOT"
fi
# Check if MIN_OSX_VERSION is set and not empty
if [[ $MIN_OSX_VERSION && ${MIN_OSX_VERSION-_} ]]; then
  C_FLAGS="$C_FLAGS -mmacosx-version-min=$MIN_OSX_VERSION"
  # clang and llvm-gcc look at mmacosx-version-min to determine link target
  # and CRT version, and use it to set the macosx_version_min linker flag
  LDFLAGS="$LDFLAGS -mmacosx-version-min=$MIN_OSX_VERSION"
fi
# Force using libc++ since it has better C++11 support required by the game
# but pre-Mavericks still use libstdc++ by default
# Also enable c++0x for consistency with the game build
C_FLAGS="$C_FLAGS -arch $ARCH -fvisibility=hidden"
LDFLAGS="$LDFLAGS -arch $ARCH -stdlib=libc++"

CFLAGS="$CFLAGS $C_FLAGS"
CXXFLAGS="$CXXFLAGS $C_FLAGS -stdlib=libc++ -std=c++0x"
OBJCFLAGS="$OBJCFLAGS $C_FLAGS"
OBJCXXFLAGS="$OBJCXXFLAGS $C_FLAGS"

JOBS=${JOBS:="-j2"}

set -e

die()
{
  echo ERROR: $*
  exit 1
}

download_lib()
{
  local url=$1
  local filename=$2

  if [ ! -e $filename ]; then
    echo "Downloading $filename"
    curl -L -o ${filename} ${url}${filename} || die "Download of $url$filename failed"
  fi
}

already_built()
{
  echo -e "Skipping - already built (use --force-rebuild to override)"
}

# Check that we're actually on OS X
if [ "`uname -s`" != "Darwin" ]; then
  die "This script is intended for OS X only"
fi

# Parse command-line options:
force_rebuild=false

for i in "$@"
do
  case $i in
    --force-rebuild ) force_rebuild=true;;
    -j* ) JOBS=$i ;;
  esac
done

cd "$(dirname $0)"
# Now in libraries/osx/ (where we assume this script resides)

# Early exit: we rely on brew to keep some 3rd party libraries up-to-date
# So if it's not installed instruct the user to do it himself (wouldn't want to force it)

update_brew()
{
  set +e
  out="$(brew install $1 2>&1)"
  if (grep -q "upgrade" <<< "${out}") ; then out="$(brew upgrade $1 2>&1)"; fi
  if !(grep -q "Warning" <<< "${out}") ; then echo "Installed $1 using homebrew."; brew cleanup $1 2>&1 1> /dev/null; else echo "$1 already up to date, skipping install."; fi
  set -e
}

symlink_brew()
{
  rm -df $1
  # path is harcoded to the default homebrew path. Assuming here that if you know how to reasonably change that, you'll be able to change this too.
  ln -s /usr/local/opt/$2/ $1
}

# Go through the libraries, and install/upgrade them using homebrew.
which -s brew
if [[ $? != 0 ]] ; then
  die "Please install Homebrew (http://brew.sh). Homebrew is a simple OSX package manager."
else
    for lib in "${BREWED_LIBRARIES[@]}"
    do
      update_brew $lib
    done
fi

# --------------------------------------------------------------
echo -e "Symlinking zlib from Brew..."

symlink_brew zlib lzlib

# --------------------------------------------------------------
echo -e "Symlinking libcurl..."

symlink_brew libcurl curl

# --------------------------------------------------------------
echo -e "Symlinking libiconv..."

symlink_brew iconv libiconv

# --------------------------------------------------------------
echo -e "Symlinking libxml2..."

symlink_brew libxml2 libxml2

# --------------------------------------------------------------
echo -e "Symlinking SDL2..."

symlink_brew sdl2 sdl2

# --------------------------------------------------------------
echo -e "Symlinking Boost..."

symlink_brew boost boost

# --------------------------------------------------------------
echo -e "Symlinking wxWidgets..."

symlink_brew wxwidgets wxmac

# --------------------------------------------------------------
echo -e "Symlinking libpng..."

symlink_brew libpng libpng

# --------------------------------------------------------------
echo -e "Symlinking libogg..."

symlink_brew libogg libogg

# --------------------------------------------------------------
echo -e "Symlinking libvorbis..."

symlink_brew vorbis libvorbis

# --------------------------------------------------------------
echo -e "Building gloox..."

LIB_VERSION="${GLOOX_VERSION}"
LIB_ARCHIVE="$LIB_VERSION.tar.bz2"
LIB_DIRECTORY="$LIB_VERSION"
LIB_URL="http://camaya.net/download/"
echo -e "Symlinking libvorbis..."

mkdir -p gloox
pushd gloox > /dev/null

if [[ "$force_rebuild" = "true" ]] || [[ ! -e .already-built ]] || [[ .already-built -ot $LIB_DIRECTORY ]]
then
  INSTALL_DIR="$(pwd)"

  rm -f .already-built
  download_lib $LIB_URL $LIB_ARCHIVE

  rm -rf $LIB_DIRECTORY bin include lib
  tar -xf $LIB_ARCHIVE
  pushd $LIB_DIRECTORY

  # TODO: pulls in libresolv dependency from /usr/lib
  # TODO: if we ever use SSL/TLS, that will add yet another dependency...
  (./configure CFLAGS="$CFLAGS" CXXFLAGS="$CXXFLAGS" LDFLAGS="$LDFLAGS" --prefix="$INSTALL_DIR" --enable-shared=no --with-zlib="${ZLIB_DIR}" --without-libidn --without-gnutls --without-openssl --without-tests --without-examples && make ${JOBS} && make install) || die "gloox build failed"
  popd
  touch .already-built
else
  already_built
fi
popd > /dev/null

# --------------------------------------------------------------
echo -e "Symlinking NSPR..."

symlink_brew nspr nspr

# --------------------------------------------------------------
echo -e "Symlinking ICU..."

symlink_brew icu icu4c

# --------------------------------------------------------------
echo -e "Symlinking ENet..."

symlink_brew enet enet

# --------------------------------------------------------------
echo -e "Symlinking MiniUPnPc..."

symlink_brew miniupnpc miniupnpc

# --------------------------------------------------------------
echo -e "Symlinking libsodium..."

symlink_brew libsodium libsodium

echo -e "Symlinking fftw..."

symlink_brew fftw fftw
# --------------------------------------------------------------------
# The following libraries are shared on different OSes and may
# be customized, so we build and install them from bundled sources
# --------------------------------------------------------------------
echo -e "Building Spidermonkey..."
LIB_VERSION="mozjs-38.2.1"
LIB_ARCHIVE="$LIB_VERSION.rc0.tar.bz2"
LIB_DIRECTORY="mozjs-38.0.0"

pushd ../source/spidermonkey/ > /dev/null

if [[ "$force_rebuild" = "true" ]] || [[ ! -e .already-built ]] || [[ .already-built -ot $LIB_DIRECTORY ]] || [[ .already-built -ot README.txt ]]
then
  INSTALL_DIR="$(pwd)"
  INCLUDE_DIR_DEBUG=$INSTALL_DIR/include-unix-debug
  INCLUDE_DIR_RELEASE=$INSTALL_DIR/include-unix-release

  rm -f .already-built
  rm -f lib/*.a
  rm -rf $LIB_DIRECTORY $INCLUDE_DIR_DEBUG $INCLUDE_DIR_RELEASE
  tar -xf $LIB_ARCHIVE

  # Apply patches
  pushd $LIB_DIRECTORY
  . ../patch.sh
  popd

  pushd $LIB_DIRECTORY/js/src
  # We want separate debug/release versions of the library, so change their install name in the Makefile
  perl -i.bak -pe 's/(^STATIC_LIBRARY_NAME\s+=).*/$1'\''mozjs38-ps-debug'\''/' moz.build

  CONF_OPTS="--target=$ARCH-apple-darwin --prefix=${INSTALL_DIR} --with-system-nspr --with-nspr-prefix=${NSPR_DIR} --with-system-zlib=${ZLIB_DIR} --disable-tests --disable-shared-js"
  # Change the default location where the tracelogger should store its output, which is /tmp/ on OSX.
  TLCXXFLAGS='-DTRACE_LOG_DIR="\"../../source/tools/tracelogger/\""'
  # Uncomment this line for 32-bit 10.5 cross compile:
  #CONF_OPTS="$CONF_OPTS --target=i386-apple-darwin9.0.0"
  if [[ $MIN_OSX_VERSION && ${MIN_OSX_VERSION-_} ]]; then
    CONF_OPTS="$CONF_OPTS --enable-macos-target=$MIN_OSX_VERSION"
  fi
  if [[ $SYSROOT && ${SYSROOT-_} ]]; then
    CONF_OPTS="$CONF_OPTS --with-macosx-sdk=$SYSROOT"
  fi

  mkdir -p build-debug
  pushd build-debug
  (CC="clang" CXX="clang++" CXXFLAGS="${TLCXXFLAGS}" AR=ar CROSS_COMPILE=1 ../configure $CONF_OPTS --enable-debug --disable-optimize --enable-js-diagnostics --enable-gczeal && make ${JOBS}) || die "Spidermonkey build failed"
  # js-config.h is different for debug and release builds, so we need different include directories for both
  mkdir -p $INCLUDE_DIR_DEBUG
  cp -R -L dist/include/* $INCLUDE_DIR_DEBUG/
  cp dist/lib/*.a $INSTALL_DIR/lib
  popd
  mv moz.build.bak moz.build

  perl -i.bak -pe 's/(^STATIC_LIBRARY_NAME\s+=).*/$1'\''mozjs38-ps-release'\''/' moz.build
  mkdir -p build-release
  pushd build-release
  (CC="clang" CXX="clang++" CXXFLAGS="${TLCXXFLAGS}" AR=ar CROSS_COMPILE=1 ../configure $CONF_OPTS --enable-optimize && make ${JOBS}) || die "Spidermonkey build failed"
  # js-config.h is different for debug and release builds, so we need different include directories for both
  mkdir -p $INCLUDE_DIR_RELEASE
  cp -R -L dist/include/* $INCLUDE_DIR_RELEASE/
  cp dist/lib/*.a $INSTALL_DIR/lib
  popd
  mv moz.build.bak moz.build
  
  popd
  touch .already-built
else
  already_built
fi
popd > /dev/null

# --------------------------------------------------------------
# NVTT - no install
echo -e "Building NVTT..."

pushd ../source/nvtt > /dev/null

if [[ "$force_rebuild" = "true" ]] || [[ ! -e .already-built ]]
then
  rm -f .already-built
  rm -f lib/*.a
  pushd src
  rm -rf build
  mkdir -p build

  pushd build

  # Could use CMAKE_OSX_DEPLOYMENT_TARGET and CMAKE_OSX_SYSROOT
  # but they're not as flexible for cross-compiling
  # Disable optional libs that we don't need (avoids some conflicts with MacPorts)
  (cmake .. -DCMAKE_LINK_FLAGS="$LDFLAGS" -DCMAKE_C_FLAGS="$CFLAGS" -DCMAKE_CXX_FLAGS="$CXXFLAGS" -DCMAKE_BUILD_TYPE=Release -DBINDIR=bin -DLIBDIR=lib -DGLUT=0 -DGLEW=0 -DCG=0 -DCUDA=0 -DOPENEXR=0 -DJPEG=0 -DPNG=0 -DTIFF=0 -G "Unix Makefiles" && make clean && make nvtt ${JOBS}) || die "NVTT build failed"
  popd

  mkdir -p ../lib
  cp build/src/nv*/libnv*.a ../lib/
  cp build/src/nvtt/squish/libsquish.a ../lib/
  popd
  touch .already-built
else
  already_built
fi
popd > /dev/null

# --------------------------------------------------------------
# FCollada - no install
echo -e "Building FCollada..."

pushd ../source/fcollada > /dev/null

if [[ "$force_rebuild" = "true" ]] || [[ ! -e .already-built ]]
then
  rm -f .already-built
  rm -f lib/*.a
  pushd src
  rm -rf output
  mkdir -p ../lib

  # The Makefile refers to pkg-config for libxml2, but we
  # don't have that (replace with xml2-config instead)
  sed -i.bak -e 's/pkg-config libxml-2.0/xml2-config/' Makefile
  (make clean && CXXFLAGS=$CXXFLAGS make ${JOBS}) || die "FCollada build failed"
  # Undo Makefile change
  mv Makefile.bak Makefile
  popd
  touch .already-built
else
  already_built
fi
popd > /dev/null
