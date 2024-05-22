#!/usr/bin/env sh

set -e

mkdir -p ./fs/usr/bin

cd cmake-build-gcc || exit

ninja init
mv usr/init ../fs/init

ninja credits
mv usr/credits ../fs/usr/bin/credits

ninja slides
mv usr/slides ../fs/usr/bin/slides

cd ../tools || exit
./create-fs.sh ../fs.bin
