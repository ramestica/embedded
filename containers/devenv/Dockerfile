FROM ubuntu:24.04

LABEL description="TM4C123 embedded dev: arm-none-eabi-gcc + OpenOCD"

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
    # Toolchain
    gcc-arm-none-eabi \
    binutils-arm-none-eabi \
    libnewlib-arm-none-eabi \
    libstdc++-arm-none-eabi-newlib \
    # Debugger / flasher
    openocd \
    # Build system
    cmake \
    ninja-build \
    make \
    # Utilities
    git \
    wget \
    curl \
    python3 \
    python3-pip \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /work
