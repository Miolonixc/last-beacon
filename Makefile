CXX ?= c++
OUT ?= build/native
CXXFLAGS ?= -O2 -std=c++17 -Wall -Wextra -Wpedantic -Werror
SDL_CFLAGS := $(shell sdl2-config --cflags)
SDL_LIBS ?= $(shell sdl2-config --libs)
HEADERS := $(wildcard src/*.hpp)
.PHONY: all play test screenshots muos package clean
all: $(OUT)/last-beacon
$(OUT):
	mkdir -p $@
$(OUT)/last-beacon: src/main.cpp $(HEADERS) | $(OUT)
	$(CXX) $(CXXFLAGS) $(SDL_CFLAGS) src/main.cpp $(SDL_LIBS) -o $@
$(OUT)/world-test: tests/world_test.cpp src/world.hpp src/levels.hpp src/profile.hpp src/demo.hpp | $(OUT)
	$(CXX) $(CXXFLAGS) tests/world_test.cpp -o $@
test: $(OUT)/world-test
	$(OUT)/world-test $(OUT)/profile-test.txt
play: all
	$(OUT)/last-beacon
screenshots: all
	SDL_VIDEODRIVER=dummy $(OUT)/last-beacon --screenshot $(OUT)/play.bmp
	SDL_VIDEODRIVER=dummy $(OUT)/last-beacon --screenshot $(OUT)/title.bmp --scene title
	SDL_VIDEODRIVER=dummy $(OUT)/last-beacon --screenshot $(OUT)/pause.bmp --scene pause
	SDL_VIDEODRIVER=dummy $(OUT)/last-beacon --screenshot $(OUT)/won.bmp --scene won
	SDL_VIDEODRIVER=dummy $(OUT)/last-beacon --screenshot $(OUT)/lost.bmp --scene lost
	SDL_VIDEODRIVER=dummy $(OUT)/last-beacon --screenshot $(OUT)/level2.bmp --level 2
	SDL_VIDEODRIVER=dummy $(OUT)/last-beacon --screenshot $(OUT)/level3.bmp --level 3
muos:
	bash tools/build-muos.sh
package:
	bash packaging/package.sh
clean:
	rm -rf build/native build/muos
