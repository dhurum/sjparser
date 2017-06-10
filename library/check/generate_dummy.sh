#!/bin/bash

dir=$1
output=$2

shift
shift

for file in $@; do
  echo "#include \"${dir}/${file}\"" >> $output
done
