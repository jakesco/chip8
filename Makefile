wasm:
	mkdir -p build
	emcc src/*.c -o build/index.html -s USE_SDL=2

native:
	mkdir -p build
	clang src/*.c -o build/chip8 -lSDL2

serve:
	python -m http.server --directory build/

clean:
	rm -rf build
