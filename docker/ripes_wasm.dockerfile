# syntax=docker/dockerfile:1.6

FROM ghcr.io/niksbrn/qt-ubuntu:6.10.2 AS builder

ARG DEBIAN_FRONTEND=noninteractive
ARG EMSDK_VERSION=4.0.7
ARG QT_VERSION=6.10.2
ARG BRANCH=master

RUN apt-get update -q \
    && apt-get install -qy --no-install-recommends \
       build-essential cmake git ca-certificates xz-utils \
       python3 python3-pip python3-venv \
       libglib2.0-0 \
    && rm -rf /var/lib/apt/lists/*

RUN mkdir -p /opt/qt/${QT_VERSION} \
    && tar -xzf /tmp/qt-${QT_VERSION}-gcc_64.tar.gz -C /opt/qt/${QT_VERSION} \
    && rm /tmp/qt-${QT_VERSION}-gcc_64.tar.gz

ENV QT_HOST_PATH=/opt/qt/${QT_VERSION}/gcc_64
ENV LD_LIBRARY_PATH=${QT_HOST_PATH}/lib

# версия emsdk должна совпадать с той, под которую собран Qt for WASM,
# иначе qt-cmake падает с "mismatch of Emscripten versions"
RUN git clone https://github.com/emscripten-core/emsdk.git /opt/emsdk \
    && cd /opt/emsdk \
    && ./emsdk install ${EMSDK_VERSION} \
    && ./emsdk activate ${EMSDK_VERSION}

RUN python3 -m venv /opt/aqt-venv \
    && /opt/aqt-venv/bin/pip install --no-cache-dir --upgrade pip \
    && /opt/aqt-venv/bin/pip install --no-cache-dir aqtinstall

RUN /opt/aqt-venv/bin/aqt install-qt all_os wasm ${QT_VERSION} wasm_singlethread \
        -O /opt/qt

RUN /opt/aqt-venv/bin/aqt install-qt all_os wasm ${QT_VERSION} wasm_singlethread \
        -m qtcharts --archives qtcharts \
        -O /opt/qt

RUN chmod -R +x /opt/qt/${QT_VERSION}/wasm_singlethread/bin

ENV QT_WASM_PATH=/opt/qt/${QT_VERSION}/wasm_singlethread

RUN git clone --recursive --branch ${BRANCH} \
        https://github.com/moevm/mse1h2026-ripes.git /tmp/ripes

# Ripes линкуется к Threads::Threads, которого нет в single-threaded WASM.
# Создаём пустой INTERFACE target через обёртку над Qt toolchain.
# --no-zstd: host rcc генерит zstd-код, которого нет в wasm Qt6Core -> линкер падает.
RUN QT_TOOLCHAIN=$(find /opt/qt/${QT_VERSION}/wasm_singlethread/lib/cmake \
        -name 'qt.toolchain.cmake' | head -n1) \
    && printf '%s\n' \
        "include(\"${QT_TOOLCHAIN}\")" \
        'if(NOT TARGET Threads::Threads)' \
        '  add_library(Threads::Threads INTERFACE IMPORTED)' \
        'endif()' \
        'set(CMAKE_AUTORCC_OPTIONS "--no-zstd" CACHE STRING "" FORCE)' \
        > /opt/ripes-wasm-toolchain.cmake

# emsdk_env.sh нужно sourсить из своей папки (использует $BASH_SOURCE внутри)
RUN cd /opt/emsdk && . ./emsdk_env.sh \
    && ${QT_WASM_PATH}/bin/qt-cmake \
        -S /tmp/ripes \
        -B /tmp/ripes/build \
        -Wno-dev \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_TOOLCHAIN_FILE=/opt/ripes-wasm-toolchain.cmake \
        -DQT_HOST_PATH=${QT_HOST_PATH} \
        -DRIPES_WITH_QPROCESS=OFF \
        -DRIPES_BUILD_TESTS=OFF \
        -DVSRTL_BUILD_TESTS=OFF \
    && cmake --build /tmp/ripes/build --parallel "$(nproc)"

RUN mkdir -p /opt/ripes-web \
    && cd /tmp/ripes/build \
    && cp -v Ripes.html Ripes.js Ripes.wasm /opt/ripes-web/ \
    && for f in qtloader.js qtlogo.svg; do \
         if [ -f "$f" ]; then cp -v "$f" /opt/ripes-web/; fi; \
       done \
    && cp /opt/ripes-web/Ripes.html /opt/ripes-web/index.html


FROM nginx:alpine AS runtime

COPY --from=builder /opt/ripes-web/ /usr/share/nginx/html/

RUN printf '%s\n' \
    'gzip on;' \
    'gzip_types application/javascript application/wasm text/html image/svg+xml;' \
    'types { application/wasm wasm; }' \
    > /etc/nginx/conf.d/wasm.conf

EXPOSE 80
