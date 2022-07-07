#!/bin/bash

cd "$(dirname -- "$0")"

mkdir -p data

for f in ./Tests/*.txt; do
    [ -e "$f" ] || continue
    echo " - Testing $f"
    ./../Search.exe "$f"
done