#!/bin/sh

set -e

if [ -d "jdk-11.0.11+9" ]; then
  echo "Skipping download of jdk release"
else
  ARCHIVE="OpenJDK11U-jdk_x64_linux_hotspot_11.0.11_9.tar.gz"
  wget "https://github.com/AdoptOpenJDK/openjdk11-binaries/releases/download/jdk-11.0.11%2B9/$ARCHIVE"
  tar -xf "$ARCHIVE"
  rm "$ARCHIVE"

  jimage extract --dir exploded-modules jdk-11.0.11+9/lib/modules
fi

if [ -d "jdk" ]; then
  echo "Skipping download of jdk sources"
else
  git clone https://github.com/openjdk/jdk.git
  pushd jdk
  git checkout -b SchokoVM jdk-11+9
  popd
fi


