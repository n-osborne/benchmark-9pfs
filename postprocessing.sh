#!/usr/bin/bash -x


sed -i 's/\x00//g' $1
sed -i '/^\s*$/d' $1
sort -n $1 -o $1
