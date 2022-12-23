#!/bin/bash

ROOT_DIR=`pwd`
TEST_DIR=${ROOT_DIR}/test
BUILD_DIR=${ROOT_DIR}/build

function clean {
  echo "-- Clean build directory"
  rm -rf $BUILD_DIR 2> /dev/null
}

function build {
  echo "-- Build start"
  mkdir $BUILD_DIR && cd $BUILD_DIR
  cmake .. -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=1
  make
  if [ ! $? -eq 0 ]; then
    echo "[Error] build failed"
    exit 1
  fi
}

clean
build

