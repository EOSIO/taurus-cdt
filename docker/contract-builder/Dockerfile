ARG EOS_CONTAINER_TAG=latest
FROM blockone-b1x-taurus-docker-local.jfrog.io/taurus/taurus-node/pinned/binary:${EOS_CONTAINER_TAG}

#CDT
COPY *.deb /
ENV DEBIAN_FRONTEND=noninteractive
RUN apt-get update -y && apt-get install -y curl pgp && \
    curl -L https://apt.llvm.org/llvm-snapshot.gpg.key |  apt-key add - && \
    echo 'deb http://apt.llvm.org/bionic/ llvm-toolchain-bionic-13 main' | tee /etc/apt/sources.list.d/llvm.list >/dev/null && \
    apt-get update -y && \
    apt-get install -y clang-13 libclang-13-dev lld-13 && \
    apt-get install -y /*.deb && \
    update-alternatives --install /usr/bin/cc cc /usr/lib/llvm-13/bin/clang 50 && \
    update-alternatives --install /usr/bin/c++ c++ /usr/lib/llvm-13/bin/clang++ 50 && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

    