#!/bin/sh
set -e
cd "$(dirname "$0")"

rm -rf glfw
git clone --branch 3.3.4 https://github.com/glfw/glfw.git

cd glfw
printf "glfw "      > git-revision.txt
git rev-parse HEAD >> git-revision.txt
rm -rf .git docs examples tests
