BUILD_FOLDER=$(CURDIR)/build
INSTALL_FOLDER=$(CURDIR)/install

CMAKE=cmake
MY_CMAKE_FLAGS=-DCMAKE_INSTALL_PREFIX="$(INSTALL_FOLDER)" -G "Ninja"

default: debug

check: check_debug

debug: MY_BUILD_FOLDER=$(BUILD_FOLDER)/debug
debug: MY_CMAKE_FLAGS+=-DCMAKE_BUILD_TYPE=Debug
debug: _build

release: MY_BUILD_FOLDER=$(BUILD_FOLDER)/release
release: MY_CMAKE_FLAGS+=-DCMAKE_BUILD_TYPE=Release
release: _build

_build: _call_cmake
	@cd $(MY_BUILD_FOLDER) && ninja install

_call_cmake:
	@mkdir -p $(MY_BUILD_FOLDER)
	cd $(MY_BUILD_FOLDER) && $(CMAKE) $(MY_CMAKE_FLAGS) $(CURDIR)

clean:
	@rm -Rf $(BUILD_FOLDER) $(INSTALL_FOLDER)

check_debug: debug
	@$(INSTALL_FOLDER)/test/strm_test_debug

check_release: release
	@$(INSTALL_FOLDER)/test/strm_test_release
