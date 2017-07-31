#!/bin/bash
killall zephyria
args=$(<args.txt)
set -e
./whitgl/scripts/build.sh  $args
(cd build/out/Zephyria.app/Contents/MacOS; ./zephyria hotreload $args "$@" &)
