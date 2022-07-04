native: src/chip.c
	clang -Wall src/*.c -o chip8 -lSDL2 -lm

run:
	./chip8

wasm: src/chip.c
	emcc src/*.c -o docs/chip8.js -s USE_SDL=2

serve:
	python -m http.server --directory docs/

clean:
	rm chip8
