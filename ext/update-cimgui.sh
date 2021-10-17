#!/bin/sh
set -e
cd "$(dirname "$0")"

rm -rf cimgui
git clone --recursive https://github.com/cimgui/cimgui.git

cd cimgui
printf "cimgui "             > git-revision.txt
git rev-parse HEAD          >> git-revision.txt
printf "imgui  "            >> git-revision.txt
git -C imgui rev-parse HEAD >> git-revision.txt
rm -rf .git .github .travis.yml .gitmodules test backend_test generator

cd imgui
rm -rf .git .github .editorconfig docs examples
rm -rf misc/cpp misc/debuggers misc/fonts/*.ttf misc/single_file misc/freetype
