#!/bin/bash

if [[ $# -eq 0 ]]; then
  echo [Error] file is not specified
  exit 1
fi

objdump -d -S -M intel $1

