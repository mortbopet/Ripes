## Build docker image

To build latest version of Ripes from `master` branch issue the command

```bash
docker build --rm --tag ripes -f ripes.dockerfile .
```

Or to build from specific branch

```bash
docker build --rm --build-arg BRANCH=my_branch --tag ripes:latest -f ripes.dockerfile .
```

## Run docker container

Enable external connection to your X server (run for each session)
```bash
xhost local:root
```
**Note:** you may add this line to `~/.xsessionrc` file to avoid having to run it for each session

After that, issue the command

```bash
docker run --rm -it --net=host -e DISPLAY=$DISPLAY -v /tmp/.X11-unix:/tmp/.X11-unix ripes:latest
```

## Web (WebAssembly) build

Ripes can be cross-compiled to WebAssembly using Qt for WASM and served via
nginx. Everything is wired up in `docker-compose.yml` as the `ripes-web` service.

Build and start:

```bash
docker compose build ripes-web
docker compose up -d ripes-web
```

Then open http://localhost:8080 in a modern browser. The browser loads
`Ripes.html` + `Ripes.js` + `Ripes.wasm` and renders the whole UI on a `<canvas>`
via WebGL.

To rebuild from a different branch:

```bash
docker compose build --build-arg BRANCH=my_branch ripes-web
```

Notes on the WASM build:
- Compiled with `RIPES_WITH_QPROCESS=OFF` and `RIPES_BUILD_TESTS=OFF` —
  `QProcess` and `QTest` are not available in the browser sandbox.
- Uses Qt's `wasm_singlethread` kit (no SharedArrayBuffer / no COOP+COEP needed).
- emsdk is pinned to `3.1.70`, which matches Qt 6.10.
