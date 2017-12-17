#!/bin/bash

output=$1

shift
shift

for file in $@; do
  echo "#include \"${file}\"" >> $output
done
