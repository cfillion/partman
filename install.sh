#!/bin/sh

root="$(realpath -q "$1")"

cd $(dirname $0)
src="$(pwd)"

mkdir -pv "$root/parts"

rsync -a "$src/src/" "$root/partman"

for file in skel/*; do
  cp -i "$src/$file" "$root/$(basename "$file")"
done
