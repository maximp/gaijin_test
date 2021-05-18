all: build

build: config
	cd _debug && make

config: _debug
	cd _debug && cmake -DCMAKE_BUILD_TYPE=Debug ..

_debug:
	mkdir _debug
