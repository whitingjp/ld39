#!/bin/bash
killall game
args=$(<args.txt)
set -e
./whitgl/scripts/build.sh  $args
(cd build/out/Game.app/Contents/MacOS; ./game hotreload $args "$@" &)
