FROM moflow/afl-tools

MAINTAINER mislav.novakovic@sartura.hr

RUN \
    apt-get update && apt-get install -y \
    # general tools
    git \
    cmake \
    build-essential \
    vim \
    # libyang
    libpcre3-dev \
    libcmocka-dev

RUN mkdir /opt/dev
WORKDIR /opt/dev

# libyang
RUN \
    git clone -b devel https://github.com/CESNET/libyang.git && \
    cd libyang && mkdir build && cd build && \
    git fetch origin pull/702/head:fuzz && git checkout fuzz && \
    cmake -DCMAKE_C_COMPILER=afl-clang-fast -DCMAKE_BUILD_TYPE="Release" -DENABLE_BUILD_FUZZ_TARGETS=ON .. && \
    make && \
    make install && \
    ldconfig
