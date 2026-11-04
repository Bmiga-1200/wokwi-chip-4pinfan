# Skills & Operational Commands: 4-Pin Fan Chip

## Local Compilation
```bash
make
# Or via wasi-sdk directly:
/opt/wasi-sdk/bin/clang --target=wasm32-unknown-wasi \
  --sysroot /opt/wasi-sdk/share/wasi-sysroot \
  -nostartfiles -Wl,--import-memory -Wl,--export-table -Wl,--no-entry -Werror \
  -o dist/4pinfan.chip.wasm src/4pinfan.chip.c
```

## Running Automated Tests
```bash
make test
```

## Tagging & Releasing
```bash
git tag -a v1.0.0 -m "Release v1.0.0"
git push origin v1.0.0
```
This triggers `.github/workflows/build.yaml`, creating a release with `chip.zip` containing `chip.json`, `chip.wasm`, `4pinfan.chip.json`, and `4pinfan.chip.wasm`.
