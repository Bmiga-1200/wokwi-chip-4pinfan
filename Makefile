SOURCES = src/4pinfan.chip.c
TARGET  = dist/4pinfan.chip.wasm

.PHONY: all
all: $(TARGET) dist/4pinfan.chip.json

.PHONY: clean
clean:
		rm -rf dist

dist:
		mkdir -p dist

$(TARGET): dist $(SOURCES) src/wokwi-api.h
	  /opt/wasi-sdk/bin/clang --target=wasm32-unknown-wasi --sysroot /opt/wasi-sdk/share/wasi-sysroot -nostartfiles -Wl,--import-memory -Wl,--export-table -Wl,--no-entry -Werror -o $(TARGET) $(SOURCES)

dist/4pinfan.chip.json: dist 4pinfan.chip.json
	  cp 4pinfan.chip.json dist

.PHONY: test
test:
	  cd tests && \
      arduino-cli core install arduino:avr && \
	  arduino-cli lib install AVR_PWM && \
      arduino-cli compile --export-binaries --fqbn arduino:avr:uno --jobs 0 --optimize-for-debug uno-pwm-tach
