
FROM jqnfxa/ripes-wasm:latest AS wasm
FROM python:3.13-slim

ARG DEBIAN_FRONTEND=noninteractive
ARG CACHEBUST=1 

RUN apt-get update && apt-get install -y \
    wget \
    xvfb \
    x11-utils \  
    x11-apps \   
    libgl1 \           
    libxcb-xinerama0 \  
    libxcb-icccm4 \
    libxcb-image0 \
    libxcb-keysyms1 \
    libxcb-render-util0 \
    libxcb-shape0 \
    libxcb-xkb1 \
    libxkbcommon-x11-0 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app/tester
ARG RIPES_TESTER_VERSION=2.2.6

RUN wget -O Ripes.AppImage https://github.com/mortbopet/Ripes/releases/download/v${RIPES_TESTER_VERSION}/Ripes-v${RIPES_TESTER_VERSION}-linux-x86_64.AppImage && \
    chmod +x Ripes.AppImage


WORKDIR /app
COPY /moodle/server/requirements.txt requirements.txt
RUN  pip install -r requirements.txt


WORKDIR /app/tester
COPY ./docker/docker_extra/create-display.sh ./create-display.sh
RUN chmod +x ./create-display.sh

WORKDIR /app
COPY /moodle/server .

COPY --from=wasm /opt/Ripes/build/Ripes.js ./static/ripes/Ripes.js
COPY --from=wasm /opt/Ripes/build/Ripes.worker.js ./static/ripes/Ripes.worker.js
COPY --from=wasm /opt/Ripes/build/qtloader.js ./static/ripes/qtloader.js
COPY --from=wasm /opt/Ripes/build/Ripes.wasm ./static/ripes/Ripes.wasm
COPY --from=wasm /opt/Ripes/build/Ripes.html ./static/ripes/Ripes.html

COPY /resources/icons/moodle.svg ./static/moodle.svg


WORKDIR /app
ENV APPIMAGE_EXTRACT_AND_RUN=1
ENV XDG_RUNTIME_DIR=/tmp/runtime-root
ENV RUNLEVEL=3

EXPOSE 5000

ENV FLASK_APP=app.py
ENTRYPOINT ["/app/tester/create-display.sh", "flask" ]
CMD ["run", "--host=0.0.0.0"]