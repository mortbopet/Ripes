# Ripes on WebAssembly

## Developer notes

## Building Ripes for WebAssembly

Some pointers:
- [WASM CI pipeline](/home/mpetersen/Work/Ripes/docs/webassembly.md)
- [Qt for WASM](https://doc.qt.io/qt-6/wasm.html)

### Debugging

To debug the webassembly build, you can use the following workflow:
1. Checkout https://github.com/mortbopet/mortbopet.github.io (Site where Ripes is hosted from)
2. Copy the `Ripes.*` files to the `mortbopet.github.io` folder. The `Ripes.*` files are generated in the build folder.
3. Use `emrun` to serve the site:
```bash
cd mortbopet.github.io
emrun index.html --no_browser
```