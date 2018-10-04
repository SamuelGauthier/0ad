#!/bin/sh

MAPNAME="water_demo"
MAPTYPE="scenarios"

if [ -z "$1" ]; then
    echo "Usage:\n"
    echo "waterquick (b|cb|rn|r|br|e|uw)"
    echo "b - build"
    echo "cb - clean and build"
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
    elif [ $1 == cb ]; then
        cd build/workspaces/gcc;
        make clean;
        #make -j3 -C build/workspaces/gcc;
        make -j3;
    elif [ $1 == brn ]; then
        make -j3 -C build/workspaces/gcc;
        ./binaries/system/pyrogenesis;
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
        cd ./libraries/osx;
        #./libraries/osx/libraries_osx_build-osx-libs.sh -j3;
        ./libraries_osx_build-osx-libs.sh -j3;
    elif [ $1 == libsf ]; then
        #./libraries/osx/build-osx-libs.sh -j3 --force-rebuild;
        cd ./libraries/osx;
        #./libraries/osx/libraries_osx_build-osx-libs.sh -j3 --force-rebuild;
        ./libraries_osx_build-osx-libs.sh -j3 --force-rebuild;
    elif [ $1 == uw ]; then
        cd ./build/workspaces;
        ./update-workspaces.sh -j3
    fi
fi
