#!/bin/sh
set -e
./binary_to_compressed_c "$1" "$2" | grep -v '// File:' > "$3"
