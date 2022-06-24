index:
	mkdir -p build
	emcc src/*.c -o build/index.html -s USE_SDL=2

serve:
	python -m http.server --directory build/

clean:
	rm -rf build
