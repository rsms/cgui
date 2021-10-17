#!/bin/sh
set -e
cd "$(dirname "$0")"

rm -rf stb
git clone https://github.com/nothings/stb.git

cd stb
printf "stb "       > git-revision.txt
git rev-parse HEAD >> git-revision.txt
rm -rf .git .github .travis.yml deprecated docs tests tools data
