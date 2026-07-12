#        _    _
#    ___| |  | |_   _  __ _  __ _ _ __
#   / _ \ |  | | | | |/ _` |/ _` | '__|
#  |  __/ |  | | |_| | (_| | (_| | |
#   \___|_|  |_|\__,_|\__, |\__,_|_|
#                       (__|
#
#  Copyright (C) 2026 rodrigo amestica
#  SPDX-License-Identifier: Apache-2.0

# Makefile — thin wrapper around CMake
BUILD_TYPE ?= Release
BUILD_DIR  := build/$(BUILD_TYPE)
PREFIX     ?= /usr/local
JOBS       ?= $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

.PHONY: all configure build install lockgen-image update-lockfiles clean distclean help

all: build

# Configure only if the build dir doesn't exist yet or CMakeCache is missing
$(BUILD_DIR)/CMakeCache.txt:
	cmake -S . -B $(BUILD_DIR) \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DCMAKE_INSTALL_PREFIX=$(PREFIX)

configure: $(BUILD_DIR)/CMakeCache.txt

build: configure
	cmake --build $(BUILD_DIR) --parallel $(JOBS)

install: build
	cmake --install $(BUILD_DIR)

lockgen-image:
	docker build --target lockgen -t devenv:lockgen -f containers/devenv/config/Dockerfile.devenv .

update-lockfiles: lockgen-image
	docker run --rm -v "$(CURDIR):/workspace" -w /workspace devenv:lockgen \
	    bash -c "cd containers/devenv/config && uv lock"
	docker run --rm -v "$(CURDIR):/workspace" -w /workspace devenv:lockgen \
	    bash -c " \
	        conan profile detect --force && \
	        cp /root/.conan2-seed-profiles/cortex-m4 /root/.conan2/profiles/cortex-m4 && \
	        conan lock create . --profile:build=default --profile:host=default --lockfile-out=conan.lock && \
	        conan lock create . --profile:build=default --profile:host=cortex-m4 --lockfile=conan.lock --lockfile-out=conan.lock --lockfile-partial \
	    "

clean:
	cmake --build $(BUILD_DIR) --target clean

distclean:
	rm -rf build

help:
	@echo "Targets:"
	@echo "  all        - configure + build (default)"
	@echo "  build      - build using BUILD_TYPE=$(BUILD_TYPE)"
	@echo "  install    - build then install to PREFIX=$(PREFIX)"
	@echo "  clean      - clean object files via CMake"
	@echo "  distclean  - remove the entire build/ directory"
	@echo ""
	@echo "Override with: make BUILD_TYPE=Debug PREFIX=/opt/foo"

#___oOo___
