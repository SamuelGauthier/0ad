==================
Build Instructions
==================

.. sectnum::

.. contents:: Table of Contents

This page describes how to get the very latest unstable version of the code.
Unless you want to actively follow and contribute to development, **you probably
want the** `latest relatively-stable release <https://play0ad.com/download>`_
**instead.**

The current release of the game is aimed at developers and not at 'normal'
users. As such, the following instructions assume a reasonable level of
technical proficiency. If you encounter difficulties, please post on the
`forum <https://www.wildfiregames.com/forum/index.php?showforum=312>`_.

General prerequisites
=====================

You'll need:

* An adequately high-spec computer:

  * At least 5GB of free disk space

  * At least 1GB of RAM for compiling, more if compiling multiple jobs in
    parallel (using -j)

  * 32 or 64-bit x86-compatible CPU, or an ARMv5+ processor

  * Modern graphics hardware is also recommended, though the game can run
    (slowly) on fairly old devices (!GeForce 4, Intel 945GM, etc)

* One of the following operating systems:

  * `Windows`_ (7 or newer)

  * `Linux`_ Linux

  * `MacOS`_ (10.7/Lion or newer)

  * `BSD`_ FreeBSD/OpenBSD (only experimental support at this time)

* Up-to-date system software (Windows service packs, graphics driver updates,
  etc)

* Some technical proficiency. We try to make the build process as smooth and
  painless as possible, but it's designed to be followed by programmers - if you
  just want to play the game, wait for a pre-packaged installer instead.

Windows
=======

The main supported versions are:

* Windows 10

* Windows 8.1

* Windows 8

* Windows 7

If you want to develop, the only supported IDEs are:

* Visual C++ 2013 (default compiler, used in official builds of the game)

* Visual C++ 2015 (recommended for developers as we will soon drop support for
  VS 2013)

**Important notes:**

* We have dropped support for older versions of Visual Studio when moving to
  C++11, see #2669.

* XP and Vista are supported as targets, but not for installing Visual Studio
  2013/2015.

* Only 32-bit builds are supported, though they can be compiled and run on
  64-bit Windows.

Acquiring the code
------------------

.. caution:: In order to get the current source code use either git with
             ``https://github.com/samuelgauthier/0ad.git`` or download the zip
             file from https://github.com/samuelgauthier/0ad

The game's code, data and build environment are stored on a Subversion server.
The recommended way to get an up-to-date copy is with TortoiseSVN:

* Download and install `TortoiseSVN <https://tortoisesvn.net/>`_. (Make sure you
  reboot when it asks you to.)

* Use TortoiseSVN to check out https://svn.wildfiregames.com/public/ps/trunk/_.
  This may take a while, and will use around 5GB of disk space. If there are
  errors during the checkout, use TortoiseSVN's "update" to resume downloading.

The `TortoiseSVN manual
<https://tortoisesvn.net/docs/release/TortoiseSVN_en/index.html>`_ has
information on `checking out
<https://tortoisesvn.net/docs/release/TortoiseSVN_en/tsvn-dug-checkout.html>`_,
as well as `updating
<https://tortoisesvn.net/docs/release/TortoiseSVN_en/tsvn-dug-update.html>`_ and
`creating patches
<https://tortoisesvn.net/docs/release/TortoiseSVN_en/tsvn-dug-patch.html>`_.

(This is the read-only public SVN URL. If you have commit access, you need to
use ``https://svn.wildfiregames.com/svn/ps/trunk/`` instead.)

If you only wish to play the most cutting-edge version, this is all you have to
do (other than `windows keepin up to date`_). The executable will be
located at ``binaries/system/pyrogenesis.exe``. 

Setting up the build environment
--------------------------------

The game must be compiled with Microsoft Visual C++. You can get the free 2015
Community edition, or 2013 Express edition, here: `Visual Studio older downloads
<https://www.visualstudio.com/vs/older-downloads/>`_. You can also install
Visual Studio 2017 and choose to install the 2015 compiler (version 14.0).

The Visual Studio project/solution files are automatically generated from the
source files:

* Run ``cd build\workspaces`` and then ``update-workspaces.bat``.
* Open ``build\workspaces\vc2015\pyrogenesis.sln`` (or ``vc2013`` for the older
  version).

Build configuration
-------------------

Make sure to select the "Release" configuration to build an optimized, more
playable version of the game (the target will be ``pyrogenesis.exe``). The
"Debug" configuration can be more useful for debugging but has significantly
reduced performance (the target will be ``pyrogenesis_dbg.exe``). Both "Release"
and "Debug" builds include debug symbols, see `Debugging
<https://trac.wildfiregames.com/wiki/Debugging>`_ and `Debugging on Windows
<https://trac.wildfiregames.com/wiki/DebuggingOnWindows>`_ for more details on
debugging.

Now you should be able to build the code from within Visual Studio, using "Build
Solution" (F7).

Building Atlas
--------------

If you also wish to test the `Atlas Scenario Editor
<https://trac.wildfiregames.com/wiki/Atlas_Manual>`_ or `Actor Editor
<https://trac.wildfiregames.com/wiki/Actor_Editor>`_ tools, you will need to
download and build the `wxWidgets <http://www.wxwidgets.org/>`_ library
separately (see ``libraries/wxwidgets/README.txt`` for details), then supply the
``--atlas`` option when running ``update-workspaces.bat``. Atlas projects will
now be included when you open ``pyrogenesis.sln`` in Visual C++.

Running
-------

Run the game with F5 inside Visual Studio (assuming "pyrogenesis" is set as the
startup project, which is default). If you want to run it outside the debugger,
run ``binaries/system/pyrogenesis_dbg.exe``.

To run the automated tests, run the "test" project. (Right click on "test" and
"set as !StartUp Project" and F5; or right click, "Debug", "Start new
instance"). In VS's debug output window, ignore any "first-chance exception"
messages; it should say ".......OK!" if it succeeded.

.. _windows keepin up to date:

Keeping up to Date
------------------

After you've set everything up, the process for staying up to date is:

* `Update
  <https://tortoisesvn.net/docs/release/TortoiseSVN_en/tsvn-dug-update.html>`_
  the root directory of the checkout.

  .. caution:: In order to get the current source code use git
               with ``https://github.com/samuelgauthier/0ad.git``

* Close the solution in Visual Studio if you've got it open. Run
  ``update-workspaces.bat`` again. (This is only needed if any source files have
  been added or removed. If you forget to run this, you'll probably get build
  errors about missing symbols.)

* Build again.

Linux
=====

0 A.D. should work on any reasonably modern Linux distro, on x86 and x86_64
(amd64). The details depend on exactly which distro you use.

Dependencies
------------

First you need to install various standard tools and development libraries:

* Boost (at least 1.57 since r21726)

* CMake (only needed if you use bundled NVTT)
* GCC (at least 4.8.1, required by C++11 features)
* fftw
* libcurl
* libenet (1.3, the older 1.2 is not compatible)
* libgloox (needed for the lobby; at least 1.0.10, previous versions are know to
  have connection problems; pass ``--without-lobby`` to ``update-workspaces.sh``
  to exclude the lobby)
* libicu
* libnspr4
* libogg
* libpng
* libsodium (>= 1.0.14, follow the instructions at
  https://download.libsodium.org/doc/installation/ if your distro is behind)
* libvorbis
* libxcursor
* libxml2
* miniupnpc (at least 1.6)
* OpenAL
* OpenGL
* SDL2 (at least 2.0.2)
* Subversion (or git if you want to use the Git mirror; see below)
* zlib

To compile editing tools (enabled by default; pass the flag ``--disable-atlas``
to ``update-workspaces.sh`` to disable):

* wxWidgets (packages are probably called wxgtk)

To use shared system libraries instead of bundled copies (default) of libraries
(pass the flag ``--with-system-$COMPONENT`` to ``update-workspaces.sh`` to use
the non-bundled copy):

* `SpiderMonkey 38 <https://developer.mozilla.org/docs/SpiderMonkey/38>`_
  (``--with-system-mozjs38``)
* `NVTT <https://github.com/castano/nvidia-texture-tools>`_
  (``--with-system-nvtt``)

For a list of all options to ``update-workspaces.sh`` see `premake
<https://trac.wildfiregames.com/wiki/premake>`_.

Debian / Ubuntu
+++++++++++++++

* On **Debian 8/jessie or Ubuntu 14.04/trusty or later** install the required
  dependencies with:

  .. code:: bash

   sudo apt-get install build-essential libboost-dev libboost-filesystem-dev \
   libcurl4-gnutls-dev libenet-dev libgloox-dev libicu-dev \
   libminiupnpc-dev libnspr4-dev libnvtt-dev libogg-dev libopenal-dev \
   libpng-dev libsdl2-dev libvorbis-dev libwxgtk3.0-dev libxcursor-dev \
   libxml2-dev subversion zlib1g-dev fftw

  * With these dependencies you have to run: ``./update-workspaces.sh
    --with-system-nvtt``

* On all versions **except Ubuntu 18.04**, you will need to `install libsodium
  manually <https://download.libsodium.org/doc/installation/>`_.

  * On **Ubuntu 18.04**, ``sudo apt-get install libsodium-dev``.

* If you want to use a packaged **mozjs38**, available for example in `0ad.dev
  PPA <https://launchpad.net/~wfg/+archive/ubuntu/0ad.dev/+packages>`_:

  * you should replace ``libnspr4-dev`` with ``libmozjs-38-dev`` and run
    ``update-workspace.sh`` with ``--with-system-mozjs38``.

* When not using system nvidia-texture-tools, ``libnvtt-dev`` can be omitted,
  but ``cmake`` is needed to build the bundled NVTT.

* You can also use ``libcurl4-openssl-dev`` instead of ``libcurl4-gnutls-dev``
  (it's not possible to install both at once), but `note that openssl is not GPL
  compatible and the resulting binaries could not be redistributed
  <http://lintian.debian.org/tags/possible-gpl-code-linked-with-openssl.html>`_.

Mandriva
++++++++

Install the dependencies with:

.. code:: bash

   urpmi gcc-c++ python subversion zip cmake boost-devel libcurl-devel \
   libenet-devel libgloox-devel libpng-devel libsodium-devel libvorbis-devel \
   libxml2-devel libwxgtku2.8-devel openal-soft-devel libicu-devel fftw

Fedora
++++++

Install the dependencies with:

.. code:: bash

   sudo dnf install gcc-c++ python subversion zip cmake patch boost-devel \
   libcurl-devel enet-devel libpng-devel libsodium-devel libvorbis-devel \
   libxml2-devel openal-soft-devel pkgconfig SDL2-devel wxGTK-devel gloox-devel\
   libicu-devel miniupnpc-devel nspr-devel fttw

* To submit a patch for review via arcanist (`Phabricator
  <https://trac.wildfiregames.com/wiki/Phabricator>`_), php is needed:  ``dnf
  install php-cli php-xml``.

openSUSE
++++++++

Install the dependencies with:

.. code:: bash

   sudo zypper install gcc-c++ python subversion zip cmake boost-devel \
   libcurl-devel libenet-devel libpng-devel libsodium-devel libvorbis-devel \
   libxml2-devel openal-soft-devel pkg-config wxWidgets-devel libSDL2-devel \
   gloox-devel libicu-devel miniupnpc-devel fftw

ArchLinux
+++++++++
.. code:: bash

   pacman -S --needed boost cmake curl enet fftw gcc gloox icu libgl libogg \
   libpng libsodium libvorbis libxcursor libxml2 miniupnpc patch sdl2 \
   subversion wxgtk zip zlib

VoidLinux
+++++++++

.. code:: bash

   sudo xbps-install -Syv base-devel boost-devel cmake curl fftw gcc icu-devel \
   libcurl-devel libenet-devel libogg-devel libopenal-devel libpng-devel \
   libsodium-devel libvorbis-devel libXcursor libxml2 MesaLib-devel \
   miniupnpc-devel nspr-devel patch pkg-config SDL2-devel wxWidgets-devel zip \
   zlib
   
If there are issues, install more header files depending on the compiler's error
message. ``nspr-devel`` is required for building SpiderMonkey and pyrogenesis
requires ``MesaLib-devel`` to provide header files for libGL. Custom compile
``gloox`` for the Lobby or use xbps source packages or use
``update-workspaces.sh --without-lobby`` or wait until
https://github.com/voidlinux/void-packages/pull/5102 is merged. If there are
unresolved shlibs or an update breaks a package, then e.g.

.. code:: bash

   sudo xbps-install -Syv SDL2-devel dbus dbus-x11  # credit Vaelatern
   sudo xpbs-install -Su  # update, add -d for debugging, credit duncaen

.. _described above:

Getting the code
----------------

.. caution:: In order to get the current source code use git
             with ``https://github.com/samuelgauthier/0ad.git``

0 A.D. is primarily developed on SVN. To checkout the latest code from SVN, run this command:

.. code:: bash

   svn co https://svn.wildfiregames.com/public/ps/trunk/ 0ad

**Note:** Sometimes SVN stops before it has downloaded all files. You should
check that it outputs something like ``at revision rXXXX``. Otherwise run

.. code:: bash

   svn up 0ad

**Note:** Make sure that the checkout directory doesn't contain special
characters (spaces or non-ASCII characters)

There are also Git mirrors, which may be slightly less up-to-date but usually
offers faster downloads. To use a Git mirror, use one of the following commands
instead:

.. code:: bash

   git clone https://github.com/0ad/0ad.git

or

.. code:: bash

   git clone https://gitlab.com/0ad/0ad.git

.. _build:

Building
--------

Compile the code with:

.. code:: bash

   cd 0ad/build/workspaces
   ./update-workspaces.sh -j3
   cd gcc
   make -j3

* **-j3** gives the number of parallel builds to run, and should typically be
  one plus the number of CPU cores available.

* The **Release** mode builds (which are the default) are more optimised, but
  are harder to debug. Use ``make config=debug`` (and run ``pyrogenesis_dbg``)
  if you need better debugging support. See `Debugging
  <https://trac.wildfiregames.com/wiki/Debugging>`_ for more details.

If you encounter any build errors, review the `existing bug reports
<https://trac.wildfiregames.com/report>`_, check the `Known Problems and
Solutions`_ or please file a `new bug in the tracker
<https://trac.wildfiregames.com/newticket>`_.

Testing
-------

Run the automated tests to verify that everything works as expected like this:

.. code:: bash

   cd ../../..
   binaries/system/test

Running
-------

If everything went well, compiling the code worked and all tests passed, it's
finally time to run the game:

.. code:: bash

   binaries/system/pyrogenesis


.. _linux keeping up to date:

Keeping up to Date
------------------

.. caution:: In order to get the current source code use git
             with ``https://github.com/samuelgauthier/0ad.git``

If you want to rebuild quickly after updating from SVN, you can usually get away
with:

.. code:: bash

   svn up
   cd build/workspaces
   ./update-workspaces.sh -j3
   cd gcc
   make -j3

If the ``make`` line gives errors, you may need to run ``make clean`` before it.
If the ``update-workspaces.sh`` gives errors, you may need to run
``clean-workspaces.sh`` before it.

Creating Linux packages
=======================

If you want to create packages for a Linux distribution see the current `0ad
<https://anonscm.debian.org/viewvc/pkg-games/packages/trunk/0ad/debian/>`_ and
`0ad-data
<https://anonscm.debian.org/viewvc/pkg-games/packages/trunk/0ad-data/debian/>`_
packages on OBS for examples (especially the ``control`` and ``rules`` files).

MacOS
=====

We recommend using the `New Way`_ over the `Old Way`_ because it is updated, is
easier and uses homebrew. Additionally in the `Old Way`_, links may be broken.

New Way
-------

* Install Xcode from the App Store

* Install the command line tools:

  .. code:: bash

   xcode-select --install

* Install homebrew:

  .. code:: bash

   /usr/bin/ruby -e "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/master/install)"

* Install the required packages:

  .. code:: bash

   brew install lzlib curl libiconv libxml12 sdl2 boost wxwidgets libpng \
   libogg libvorbis nspr icu4c enet miniupnpc libsodium fftw git cmake

* Get the game source code, build the libraries and update the workspaces:

  .. code:: bash

   git clone https://github.com/samuelgauthier/0ad.git
   cd 0ad
   ./waterquick libs
   ./waterquick uw


* Build the game and run it:

  .. code:: bash

   ./waterquick br

  The script will launch the default demo map. If you would like to build and
  launch it without a map run the following instead:

  .. code:: bash

   ./waterquick brn

  If you want to change the default loaded map, edit the ``waterquick.sh``
  script and change the variables ``MAPNAME`` and ``MAPTYPE`` accordingly.

  If you just want to run the game use ``./waterquick r`` or ``./waterquick rn``

Old Way
-------

The process on OS X is similar to Linux:

* Obtain the command line tools:

  * If you're using Lion 10.7.3 or later, Apple has made their *Command Line
    Tools for Xcode* package freely available, as a separate download. **It
    does not include or require Xcode.** If you don't want the Xcode IDE, it's
    recommended to install only this package from
    `Apple Developer Downloads <https://developer.apple.com/downloads/index.action>`_ 
    . You need a free Apple ID to access the download. If you already
    have Xcode 5.1+, you can download the command line tools from the
    `download preferences
    <https://developer.apple.com/library/IOs/#recipes/xcode_help-documentation_preferences/DownloadingandInstallingXcodeComponents/DownloadingandInstallingXcodeComponents.html>`_.

  * If you're using Lion 10.7.2 or earlier, you'll probably need to install
    Xcode to get the command line tools:
    
    * If available, use your Mac OS X install DVD which saves downloading 1.72+
      GB.

    * Visit `Apple Developer Downloads
      <https://developer.apple.com/downloads/index.action>`_ (logging in with
      your free Apple ID) and download the latest Xcode version for your OS.

    * The latest version of Xcode is also available for free from the
      `Mac App Store <http://www.apple.com/mac/app-store/>`_. *Note: if you
      download the app, it is only the installer for Xcode. You need to open it
      and run the "Install Xcode" app.*

  * If you want to build a distributable app bundle as described below, you will
    need Xcode.

  * **Note:** As of Mavericks (10.9) and Xcode 5, Apple
    `no longer supports
    <https://developer.apple.com/library/mac/releasenotes/DeveloperTools/RN-Xcode/xc5_release_notes/xc5_release_notes.html#//apple_ref/doc/uid/TP40001051-CH2-SW302>`_
    llvm-gcc, instead it is required to use clang.
    Additionally, the default C++ library is now libc++ instead of libstdc++. If
    you've upgraded and previously built the game, you should pass the
    ``--force-rebuild`` flag to ``build-osx-libs.sh``.

* As of Alpha 21, the game uses SpiderMonkey 38 which requires a Python 2.7
  version later than 2.7.3. If you're on Mountain Lion (10.8) or earlier, you
  will need to first update your Python installation with the latest 2.7.x
  installer from `the official python site
  <https://www.python.org/downloads/release/python-279/>`_.

* Obtain CMake:

  * You can download a prebuilt OS X package `from cmake's site
    <https://cmake.org/download/>`_.

  * If prompted, install the CMake command line tools to the default location.

  * **Note:** Recent versions have no installer, so after copying the app
    bundle to **Applications**, you need to run CMake with elevated
    permissions to install the command line tools. From the terminal:

    .. code:: bash

       sudo "/Applications/CMake.app/Contents/bin/cmake-gui" --install

  * If the install command fails, you can manually add the following line to
    the end of ``/etc/paths``:

    .. code:: bash
     
        /Applications/CMake.app/Contents/bin

* Obtain the game's source code:

  .. caution:: In order to get the current source code use git
               with ``https://github.com/samuelgauthier/0ad.git``


  * 0 A.D. is primarily developed on SVN. To checkout the latest code from SVN, run this command:

    .. code:: bash

     svn co https://svn.wildfiregames.com/public/ps/trunk/ 0ad

    **Note:** Sometimes SVN stops before it has downloaded all files. You
    should check that it outputs something like ``at revision rXXXX``.
    Otherwise run

    **Warning** To get the source of the current repo use the git way instead.

    .. code:: bash

     svn up 0ad

  * There is also a Git mirror, which may be slightly less up-to-date but
    usually offers faster downloads. To use the Git mirror, use this command
    instead:

    .. code:: bash

      git clone https://github.com/samuelgauthier/0ad.git

Now you have two options:

Build the game for your personal use
++++++++++++++++++++++++++++++++++++

* Run ``libraries/osx/build-osx-libs.sh``, the OS X libraries build script, this
  will download and build the game's dependencies (except CMake, see above).
  This script will take some time to finish when first run, after that it will
  reuse the old build.

  .. code:: bash

   cd libraries/osx
   ./build-osx-libs.sh -j3

  * **-j3** gives the number of parallel builds to run, and should typically
    be one plus the number of CPU cores available.

  * To force a rebuild for some reason, e.g. the SVN folder is moved or Xcode /
    OS X is upgraded, pass in the ``--force-rebuild`` flag.

* Next, to build the game on the command line, use the following commands:

  .. code:: bash

   cd 0ad/build/workspaces
   ./update-workspaces.sh -j3
   cd gcc
   make -j3

  * The **Release** mode builds (which are the default) are more optimised, but
    are harder to debug. Use ``make config=debug`` (and run ``pyrogenesis_dbg``)
    if you need better debugging support. See `Debugging
    <https://trac.wildfiregames.com/wiki/Debugging>`_ for more details.

  * If you encounter any build errors, review the `existing bug reports
    <https://trac.wildfiregames.com/report>`_, check the `known problems section
    <Known Problems and Solutions>`_ or please `file a new bug in the tracker
    <https://trac.wildfiregames.com/newticket>`_.

* Or if you have Xcode 4 installed, you can open
  ``build/workspaces/xcode4/pyrogenesis.xcworkspace`` (see discussion on this
  `in this forum thread
  <https://www.wildfiregames.com/forum/index.php?showtopic=15511&st=160#entry261743>`_).

* Run the automated tests to verify that everything works as expected like this:

  .. code:: bash

      ./binaries/system/test

* If everything went well, compiling the code worked and all tests passed, it's
  finally time to run the game:

  .. code:: bash

      ./binaries/system/pyrogenesis

* **Note:** Newer versions of Xcode no longer include the command line tools by
  default, you need to install them as described above.

* **Note:** It is recommended to use the command line build, since the Xcode
  build is not as well-tested, but Xcode's IDE can be very useful for code
  editing.

Build the game as a distributable app bundle
++++++++++++++++++++++++++++++++++++++++++++

* You will need Xcode installed (for its SDKs)

* Open ``build/workspaces/build-osx-bundle.sh`` and read the comments. You will
  need to change a few settings depending on your version of OS X, Xcode, etc.

* Run ``build-osx-bundle.sh``, the bundle build script, which will download and
  build the game's dependencies for the appropriate SDK, build the game's source
  code, package the mod data, and set up the app bundle info.

  .. code:: bash

   cd build/workspaces
   ./build-osx-bundle.sh -j3

* **-j3** gives the number of parallel builds to run, and should typically be
  one plus the number of CPU cores available.

* When it's finished, there should be a complete **0ad app** bundle in
  ``build/workspaces``. You can open it by double-clicking its icon in Finder or
  with the ``open 0ad.app`` command in the terminal.

* Consider the following to make an official release:

  * Use ``build-osx-bundle.sh --release``, to create a bundle from a clean SVN
    working copy.

  * Package the bundle inside a compressed DMG with background image, for easy
    distribution (see ReleaseProcess).


BSD
===

**Note: The *BSD support is a work in progress and should be considered
experimental. That means don't try it unless you "know what you're doing" :)**

* Install the following ports or packages (names probably differ depending on
  the BSD variant):
  Install commands for the variants are provided below.

  * boost-libs
  * cmake
  * curl
  * enet
  * execinfo
  * fftw
  * gloox
  * gmake
  * iconv
  * icu
  * libGL
  * libogg
  * libvorbis
  * libxml2
  * miniupnpc
  * nspr
  * openal
  * png
  * sdl2
  * subversion
  * wxWidgets-gtk2 (unicode) - required to build the Atlas editor
  * zip
  * **Note:** zlib should already be installed by default
  * GCC 4.8.1+ or Clang
* Obtain the game's source code as `described above`_ for Linux.
* Check for any variant specific issues below.
* **Note:** Our build scripts should detect that you are running \*BSD and use
  ``gmake`` as the make command. If for some reason this isn't correct, you can
  set the ``MAKE`` environment variable to the correct GNU make path.
* Follow the `build`_ instructions above for Linux.

FreeBSD
-------

* Install the dependencies with:
  
  .. code:: bash

   pkg install boost-libs cmake curl enet fftw gloox gmake iconv libGL libogg
   libsodium libvorbis libxml2 miniupnpc nspr openal pkgconf png sdl2 subversion
   wx30-gtk2 zip

* If running FreeBSD 10.0+ you need to set ``CC`` to ``clang`` and ``CXX`` to
  ``clang++``.

  .. code:: bash

   export CC=clang CXX=clang++

* TODO: Fix missing ecvt() (see `#1325
  <https://trac.wildfiregames.com/ticket/1325>`_)

* If building Atlas, you need to set the ``WX_CONFIG`` variable, because
  ``wx-config`` has a different name on FreeBSD. For example, you'd run this
  command if you built the wxGTK 2.8 package with unicode support:

  .. code:: bash

   export WX_CONFIG=wxgtk2u-2.8-config

  if not correct, you will get errors about missing "wx/\*.h" includes. You can
  skip building Atlas altogether (and the wxWidgets dependency) by later passing
  the ``--disable-atlas`` option to ``update-workspaces.sh``.

* You'll have to set this variable every time you run ``update-workspaces.sh``,
  so it may be most convenient to put these commands into another shell script.

OpenBSD
-------

* As we require GCC 4.8.1+ you need to set ``CC`` and ``CXX`` before building

  .. code:: bash

   export CC=egcc CXX=eg++

* Install the dependencies with:

  .. code:: bash

   pkg_add -i boost cmake curl enet fftw g++ gcc gloox gmake icu4c libexecinfo
   libogg libsodium libxml miniupnpc nspr openal png sdl2 subversion zip 

* As OpenBSD's packaged libxml isn't build with threading support, building
  Atlas is not possible so you should run ``update-workspaces.sh`` with the
  ``--disable-atlas`` option.

* You probably need to run pyrogenesis with
  ``LD_PRELOAD=/usr/local/lib/libogg.so.6.2:/usr/local/lib/libvorbis.so.8.0``
  (see `#1463 <https://trac.wildfiregames.com/ticket/1463>`_).

Known Problems and Solutions
============================

* None currently.
