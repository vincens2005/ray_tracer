all: raylib build

raylib:
	 @echo building raylib...
	 cd raylib/src/ && \
	 make PLATFORM=PLATFORM_DESKTOP

	cp raylib/src/raylib.h include/
	cp raylib/src/raymath.h include/



build: raylib
	@echo building ray tracer...

	if [ ! -d "build" ]; then \
		mkdir build; \
	fi

	gcc -Wall -Lraylib/src -L/opt/vc/lib -Iinclude main.c -o build/raytracer -lraylib -lm

	@echo done!

clean:
	rm -rf build
	cd raylib/src/ && \
	make clean

run: build
	./build/raytracer

.PHONY: all clean raylib

