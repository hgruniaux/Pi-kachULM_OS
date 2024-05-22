#!/usr/bin/env sh

set -e

mkdir -p ./fs/usr/bin

cd cmake-build-release-clang-17 || exit

make init
mv usr/init ../fs/init

make credits
mv usr/credits ../fs/usr/bin/credits

make slides
mv usr/slides ../fs/usr/bin/slides

#cd ../tools || exit
#./create-fs.sh ../home_made/gen-files/fs.img
