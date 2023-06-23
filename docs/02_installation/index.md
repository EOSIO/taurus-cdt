# Building EOSIO-Taurus CDT (Contract Development Toolkit)

## Install llvm 13

### Ubuntu
```
source /etc/lsb-release
curl -L https://apt.llvm.org/llvm-snapshot.gpg.key | sudo apt-key add -
echo "deb http://apt.llvm.org/$DISTRIB_CODENAME/ llvm-toolchain-$DISTRIB_CODENAME-13 main" | sudo tee /etc/apt/sources.list.d/llvm.list >/dev/null
sudo apt-get update -y
sudo apt-get install -y clang-13 libclang-13-dev lld-13 libc++-13-dev libc++abi-13-dev
export CMAKE_PREFIX_PATH=/usr/lib/llvm-13
```

## Build

```sh
git clone <this repo>
cd taurus-cdt
mkdir build
cd build
cmake -DCMAKE_PREFIX_PATH=$CMAKE_PREFIX_PATH -DCMAKE_BUILD_TYPE=Release ..
make -j8
```

From here onward you can build your contracts code by simply exporting the `build` directory to your path, so you don't have to install globally (makes things cleaner).
Or you can install globally by running this command:

```sh
sudo make install
```

