BUILD_FOLDER=$(CURDIR)/build
PROFILER_DATA_FOLDER=$(CURDIR)/.profiler
CMAKE_GENERATOR="Ninja"
GIT_TAG=`git describe --tags --dirty --always`

export CC=clang-5.0
export CXX=clang++-5.0

build: _call_cmake

check: check_debug

benchmark: GTEST_ARGS=--gtest_filter="benchmark.*"
benchmark: check_release

docs:
	@doxygen
	@echo "Documentation index at ./doxygen/html/index.html"

clean:
	@rm -Rf $(BUILD_FOLDER)

check_debug: _profiler_folder build
	@$(BUILD_FOLDER)/strm_test_debug $(GTEST_ARGS) .profiler/$(GIT_TAG).debug.csv

check_release: _profiler_folder build
	@$(BUILD_FOLDER)/strm_test_release $(GTEST_ARGS) .profiler/$(GIT_TAG).release.csv

_profiler_folder:
	@mkdir -p $(PROFILER_DATA_FOLDER)

_call_cmake:
	@mkdir -p $(BUILD_FOLDER)
	@cd $(BUILD_FOLDER) && cmake -G $(CMAKE_GENERATOR) $(CURDIR)
	@cd $(BUILD_FOLDER) && cmake --build .
