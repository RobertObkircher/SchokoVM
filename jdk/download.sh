#!/bin/sh
set -e

if [ -d "jdk" ]; then
  echo "Skipping download of jdk sources"
else
  git clone https://github.com/openjdk/jdk.git
  pushd jdk
  git checkout -b SchokoVM jdk-11+9
  popd
fi


