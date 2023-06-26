FROM ubuntu:20.04

ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update -q \
    && apt-get install -qy --no-install-recommends \
    build-essential \
    cmake \
    gcc-riscv64-unknown-elf \
    git \
    libpthread-stubs0-dev \
    python3 \
    python3-pip \
    libegl1 libegl1-mesa-dev \
    libgl1-mesa-dev libglib2.0-dev \
    libxkbcommon-x11-0 libpulse-dev libfontconfig1-dev libfreetype6-dev \
    libxcb-icccm4 libxcb-image0 libxcb-keysyms1 libxcb-render-util0 \
    libxcb-xinerama0 libxcb-composite0 libxcb-cursor0 libxcb-damage0 \
    libxcb-dpms0 libxcb-dri2-0 libxcb-dri3-0 libxcb-ewmh2 libxcb-glx0 \
    libxcb-present0 libxcb-randr0 libxcb-record0 libxcb-render0 libxcb-res0 \
    libxcb-screensaver0 libxcb-shape0 libxcb-shm0 libxcb-sync1 libxcb-util1 \
    && apt-get -y autoremove \
    && apt-get -y autoclean \
    && rm -rf /var/lib/apt/lists/*

RUN python3 -m pip install aqtinstall

# from https://ddalcino.github.io/aqt-list-server/
RUN aqt install-qt linux desktop 6.5.0 gcc_64 -m qtcharts

ARG GIT_SSL_NO_VERIFY=true
ENV LC_ALL=C.UTF-8 SHELL=/bin/bash
ENV LD_LIBRARY_PATH="${LD_LIBRARY_PATH}:/6.5.0/gcc_64/lib"

ARG BRANCH=master
RUN git clone --recursive --branch ${BRANCH} https://github.com/mortbopet/Ripes.git /tmp/ripes \
    && cmake -S /tmp/ripes/ \
        -B /tmp/ripes/build \
        -Wno-dev            \
        -DRIPES_BUILD_TESTS=ON \
        -DVSRTL_BUILD_TESTS=ON \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_PREFIX_PATH=$(pwd)/6.5.0/gcc_64/ \
    && cmake --build /tmp/ripes/build \
    && cd /tmp/ripes/build/test \
    && ./tst_assembler && ./tst_expreval && ./tst_riscv \
    && cd /tmp/ripes/build \
    && make install \
    && cd /tmp \
    && rm -rf /tmp/ripes

ENTRYPOINT ["Ripes"]
