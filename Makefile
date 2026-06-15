.PHONY: clean triangle texture bunny

all: build/include/gl.h build/lib/libgl.so examples/triangle examples/texture examples/bunny

build/include/gl.h: include/gl.h
	mkdir -p build/include
	cp include/gl.h build/include/gl.h

build/lib/libgl.so: clip.c gl.c mat.c obj.c pipe.c rast.c tga.c
	mkdir -p build/lib
	gcc -g -shared -fPIC clip.c gl.c mat.c obj.c pipe.c rast.c tga.c -Iinclude -lm -o build/lib/libgl.so

examples/triangle: build/lib/libgl.so build/include/gl.h
	gcc -g examples/triangle.c examples/driver.c build/lib/libgl.so -Iexamples/include -Ibuild/include -Lbuild/lib -lgl -lSDL2main -lSDL2 -o examples/triangle

examples/texture: build/lib/libgl.so build/include/gl.h
	gcc -g examples/texture.c examples/driver.c build/lib/libgl.so -Iexamples/include -Ibuild/include -Lbuild/lib -lgl -lSDL2main -lSDL2 -o examples/texture

examples/bunny: build/lib/libgl.so build/include/gl.h
	gcc -g examples/bunny.c examples/driver.c build/lib/libgl.so -Iexamples/include -Ibuild/include -Lbuild/lib -lgl -lSDL2main -lSDL2 -o examples/bunny

triangle: examples/triangle build/lib/libgl.so
	LD_LIBRARY_PATH=build/lib ./examples/triangle

texture: examples/texture build/lib/libgl.so
	LD_LIBRARY_PATH=build/lib ./examples/texture

bunny: examples/bunny build/lib/libgl.so
	LD_LIBRARY_PATH=build/lib ./examples/bunny

clean:
	rm -rf build
	rm examples/triangle
	rm examples/texture
	rm examples/bunny