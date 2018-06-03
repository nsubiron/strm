BUILD_FOLDER=$(CURDIR)/build
CMAKE_GENERATOR="Ninja"

export CC=clang
export CXX=clang++

build: _call_cmake

check: check_debug

docs:
	@doxygen
	@echo "Documentation index at ./doxygen/html/index.html"

clean:
	@rm -Rf $(BUILD_FOLDER)

check_debug: build
	@$(BUILD_FOLDER)/strm_test_debug

check_release: build
	@$(BUILD_FOLDER)/strm_test_release

_call_cmake:
	@mkdir -p $(BUILD_FOLDER)
	@cd $(BUILD_FOLDER) && cmake -G $(CMAKE_GENERATOR) $(CURDIR)
	@cd $(BUILD_FOLDER) && cmake --build .
