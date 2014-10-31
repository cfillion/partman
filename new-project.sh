#!/bin/sh

root="$(realpath -q "$1")"

cd $(dirname $0)
pwd="$(pwd)"

mkdir -pv "$root/parts"

for file in skel/*; do
  ln -svi "$pwd/$file" "$root/$(basename "$file")"
done

ln -svi "$pwd" "$root/partman"
