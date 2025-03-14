CXXFLAGS += -g --std=c++23 -pedantic -Wall

BUILD_FOLDER ?= build

PROJECT_NAME = symexpr

# additional targets
ADDONS = differentiator

RELEASE ?= 0
ifeq ($(RELEASE), 0)
	BUILD_FOLDER := $(BUILD_FOLDER)/debug
else
	BUILD_FOLDER := $(BUILD_FOLDER)/release
	CXXFLAGS += -O3 -flto -DNDEBUG
endif

COVERAGE ?= 0
ifeq ($(COVERAGE), 1)
	BUILD_FOLDER := $(BUILD_FOLDER)/coverage
	CXXFLAGS += -fprofile-arcs -ftest-coverage
	POST_BUILD_COMMAND+=\
		lcov --capture --directory $(BUILD_FOLDER) --output-file $(BUILD_FOLDER)/coverage.info;\
		genhtml $(BUILD_FOLDER)/coverage.info --output-directory $(BUILD_FOLDER)/coverage-report;
endif

COMPILE = $(CXX) $(CXXFLAGS)

.phony: all test test.valgrind test.callgrind clean $(ADDONS) $(ADDONS:=.time) $(ADDONS:=.valgrind) $(ADDONS:=.callgrind)

all: $(BUILD_FOLDER)/$(PROJECT_NAME).o $(BUILD_FOLDER)/test $(ADDONS:%=$(BUILD_FOLDER)/%)

$(BUILD_FOLDER):
	mkdir -p $(BUILD_FOLDER)

# build lib object
$(BUILD_FOLDER)/$(PROJECT_NAME).o: src/$(PROJECT_NAME).cpp src/*.h | $(BUILD_FOLDER)
	$(COMPILE) $< -c -o $@

# build tests
$(BUILD_FOLDER)/tests.o: tests/tests.cpp src/*.h tests/utils.h | $(BUILD_FOLDER)
	$(COMPILE) $< -c -o $@

# link tests
$(BUILD_FOLDER)/test: $(BUILD_FOLDER)/tests.o $(BUILD_FOLDER)/$(PROJECT_NAME).o | $(BUILD_FOLDER)
	$(COMPILE) $^ -o $@

# build addons
$(ADDONS:%=$(BUILD_FOLDER)/%.o): $(BUILD_FOLDER)/%.o: src/%.cpp src/*.h | $(BUILD_FOLDER)
	$(COMPILE) $< -c -o $@

# link addons
$(ADDONS:%=$(BUILD_FOLDER)/%): $(BUILD_FOLDER)/%: $(BUILD_FOLDER)/%.o $(BUILD_FOLDER)/$(PROJECT_NAME).o | $(BUILD_FOLDER)
	$(COMPILE) $^ -o $@

RUNNABLES = $(ADDONS) test

# targets to run addons + test
$(RUNNABLES): %: $(BUILD_FOLDER)/%
	$<
	$(POST_BUILD_COMMAND)

$(RUNNABLES:=.time): %.time: $(BUILD_FOLDER)/%
	bash -c "time $<"
	$(POST_BUILD_COMMAND)

$(RUNNABLES:=.valgrind): %.valgrind: $(BUILD_FOLDER)/%
	valgrind $<
	$(POST_BUILD_COMMAND)

$(RUNNABLES:=.callgrind): %.callgrind: $(BUILD_FOLDER)/%
	valgrind --tool=callgrind --dump-instr=yes --collect-jumps=yes $<
	$(POST_BUILD_COMMAND)

clean:
	rm -rf build
