FROM ubuntu:20.04

# Set noninteractive mode for apt-get to avoid prompts
ENV DEBIAN_FRONTEND=noninteractive
ARG CACHEBUST=1

# Update and install dependencies
RUN apt-get update && \
    apt-get upgrade -y && \
    apt-get install -y -q --force-yes \
    automake cmake git wget libfuse2 desktop-file-utils \
    build-essential libgl1-mesa-dev libxkbcommon-x11-0 libpulse-dev \
    libxcb-icccm4 libxcb-image0 libxcb-keysyms1 libxcb-render-util0 \
    libxcb-xinerama0 libxcb-composite0 libxcb-cursor0 libxcb-damage0 \
    libxcb-dpms0 libxcb-dri2-0 libxcb-dri3-0 libxcb-ewmh2 libxcb-glx0 \
    libxcb-present0 libxcb-randr0 libxcb-record0 libxcb-render0 libxcb-res0 \
    libxcb-screensaver0 libxcb-shape0 libxcb-shm0 libxcb-sync1 libxcb-util1 libegl1 libegl1-mesa-dev \
    python3 python3-pip && \
    rm -rf /var/lib/apt/lists/*  # Clean up apt cache

# Install aqt (for Qt installation)
RUN python3 -m pip install aqtinstall==3.1.* py7zr>=0.20.2

# Create a directory for Qt and Ripes
WORKDIR /opt
RUN mkdir Qt Ripes

COPY ./docker/docker_extra/settings.ini .

RUN mkdir -p /usr/local/lib/python3.8/dist-packages/aqt
RUN mv settings.ini /usr/local/lib/python3.8/dist-packages/aqt/settings.ini

# Install Qt (Host)
RUN aqt install-qt linux desktop 6.6.0 gcc_64 -m qtcharts -O /opt/Qt

# Install Qt (Emscripten/WASM)
RUN aqt install-qt linux desktop 6.6.0 wasm_multithread -m qtcharts -O /opt/Qt

# Install Emscripten
WORKDIR /opt
RUN git clone https://github.com/emscripten-core/emsdk.git
WORKDIR /opt/emsdk
RUN ./emsdk install 3.1.25 && \
    ./emsdk activate 3.1.25

# Set Emscripten environment variables (important!)
ENV EMSDK /opt/emsdk
ENV EM_CONFIG $EMSDK/.emscripten
ENV PATH $EMSDK:$EMSDK/upstream/emscripten:$EMSDK/node/14.18.2_64bit/bin:$PATH
ENV CC emcc
ENV CXX em++

# Clone Ripes repository (inside the container)
WORKDIR /opt/Ripes
ARG BRANCH=master
RUN git clone --recursive --branch ${BRANCH} https://github.com/moevm/mse1h2025-ripes . \
    && rm -rf ./src/* ./resources/*

COPY ./src ./src
COPY ./resources ./resources

# Set QT_HOST_PATH
ENV QT_HOST_PATH /opt/Qt/6.6.0/gcc_64

# Create build directory
WORKDIR /opt/Ripes/build

# Install CMake 3.21 (or later) from Kitware's repository
RUN wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc 2>/dev/null | gpg --dearmor - | tee /usr/share/keyrings/kitware-archive-keyring.gpg >/dev/null \
    && echo 'deb [signed-by=/usr/share/keyrings/kitware-archive-keyring.gpg] https://apt.kitware.com/ubuntu/ focal main' | tee /etc/apt/sources.list.d/kitware.list >/dev/null \
    && apt-get update \
    && apt-get install cmake -y

# Run CMake and Make
RUN echo ". /opt/emsdk/emsdk_env.sh" >> ~/.bashrc && \
    cmake \
      -DCMAKE_BUILD_TYPE=Release \
      -DRIPES_WITH_QPROCESS=OFF \
      -DEMSCRIPTEN=1 \
      -DQT_HOST_PATH="$QT_HOST_PATH" \
      -DCMAKE_TOOLCHAIN_FILE=/opt/Qt/6.6.0/wasm_multithread/lib/cmake/Qt6/qt.toolchain.cmake \
      -DCMAKE_PREFIX_PATH=/opt/Qt/6.6.0/wasm_multithread \
      -DEMSCRIPTEN_FORCE_COMPILERS=ON \
      .. && \
    make -j$(nproc)
