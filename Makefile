all: build/include/gl.h build/libgl.so examples/triangle

build/include/gl.h:
	mkdir -p build/include
	cp include/gl.h build/include/gl.h

build/libgl.so:
	mkdir -p build
	gcc -c -fPIC clip.c gl.c mat.c obj.c pipe.c rast.c shad.c tga.c -Iinclude -o build/gl.o
	gcc -shared build/gl.o -o build/libgl.so
	rm build/gl.o

examples/triangle: build/libgl.so build/include/gl.h
	gcc -c examples/triangle.c examples/driver.c build/libgl.so -Iexamples/include -Ibuild/include -lSDL2main -lSDL2 -o examples/triangle