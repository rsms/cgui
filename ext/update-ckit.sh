#!/bin/sh
set -e
cd "$(dirname "$0")"

rm -rf ckit
git clone https://github.com/rsms/ckit.git

cd ckit
printf "ckit "      > git-revision.txt
git rev-parse HEAD >> git-revision.txt
rm -rf .git example
