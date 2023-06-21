#!/bin/bash
source /etc/lsb-release
export DEBIAN_FRONTEND=noninteractive
apt-get update
apt-get install -y curl pgp
curl -L https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -
echo "deb http://apt.llvm.org/$DISTRIB_CODENAME/ llvm-toolchain-$DISTRIB_CODENAME-13 main" | tee /etc/apt/sources.list.d/llvm.list >/dev/null
apt-get update && apt-get install -y clang-13 lld-13 libc++-13-dev libc++abi-13-dev