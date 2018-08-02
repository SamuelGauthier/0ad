#!/bin/sh

MAPNAME="water_demo"
MAPTYPE="scenarios"

if [ -z "$1" ]; then
    echo "Usage:\n"
    echo "waterquick (b|r|br|e)"
    echo "b - build"
    echo "rn - run"
    echo "r - run with loading map"
    echo "br - build and run with loading map"
    echo "e - editor"
    echo "libs - build libraries"
    echo "libsf - force build libraries"
    echo "uw - update workspaces"
else
    if [ $1 == b ]; then
        make -j3 -C build/workspaces/gcc;
    elif [ $1 == rn ]; then
        ./binaries/system/pyrogenesis;
    elif [ $1 == r ]; then
        #./binaries/system/pyrogenesis -autostart="skirmishes/Cycladic Archipelago (3)";
        echo $MAPNAME;
        ./binaries/system/pyrogenesis -autostart="$MAPTYPE/$MAPNAME";
    elif [ $1 == br ]; then
        make -j3 -C build/workspaces/gcc;
        ./binaries/system/pyrogenesis -autostart="$MAPTYPE/$MAPNAME";
    elif [ $1 == e ]; then
        ./binaries/system/pyrogenesis -editor;
    elif [ $1 == libs ]; then
        #./libraries/osx/build-osx-libs.sh -j3;
        ./libraries/osx/libraries_osx_build-osx-libs.sh -j3;
    elif [ $1 == libsf ]; then
        #./libraries/osx/build-osx-libs.sh -j3 --force-rebuild;
        ./libraries/osx/libraries_osx_build-osx-libs.sh -j3 --force-rebuild;
    fi
fi
