DEPS_CFLAGS?=$(shell env PKG_CONFIG_PATH=/usr/local/frc/lib/pkgconfig pkg-config --cflags wpilibc)
DEPS_LIBS?=$(shell env PKG_CONFIG_PATH=/usr/local/frc/lib/pkgconfig pkg-config --libs wpilibc)
EXE=vision2020
DESTDIR?=/home/pi/

.PHONY: clean build install

build: ${EXE}

install: build
	cp ${EXE} runCamera ${DESTDIR}

clean:
	rm -f ${EXE} *.o

OBJS=main.o MyPipeline.o

${EXE}: ${OBJS}
	${CXX} -pthread -g -o $@ $^ ${DEPS_LIBS} -Wl,--unresolved-symbols=ignore-in-shared-libs

.cpp.o:
	${CXX} -pthread -Wall -Wextra -pedantic -g -Og -c -o $@ -std=c++17 ${CXXFLAGS} ${DEPS_CFLAGS} $<
