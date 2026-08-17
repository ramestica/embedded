#        _    _
#    ___| |  | |_   _  __ _  __ _ _ __
#   / _ \ |  | | | | |/ _` |/ _` | '__|
#  |  __/ |  | | |_| | (_| | (_| | |
#   \___|_|  |_|\__,_|\__, |\__,_|_|
#                       (__|
#
#  Copyright (C) 2026 rodrigo amestica
#  SPDX-License-Identifier: Apache-2.0

# Makefile — thin wrapper around CMake, configured via Conan profiles.
#
# FLAVOR selects the Conan host profile / toolchain:
#   host      — native compiler, no cross toolchain (default)
#   cortex-m4 — arm-none-eabi cross toolchain (application/'s target)
BUILD_TYPE ?= Release
FLAVOR     ?= host
BUILD_DIR  := build/$(BUILD_TYPE)/$(FLAVOR)
PREFIX     ?= /usr/local
JOBS       ?= $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

CONAN_HOST_PROFILE := $(if $(filter cortex-m4,$(FLAVOR)),cortex-m4,default)
PRESET := conan-$(FLAVOR)-$(shell echo $(BUILD_TYPE) | tr '[:upper:]' '[:lower:]')

# Bare-metal targets have no libc syscall stubs until a real executable links
# one in, so CMake's default compiler probe needs relaxing — see
# cmake/toolchain/cortex-m4-user.cmake. Kept out of the checked-in cortex-m4
# profile since the absolute path only makes sense at invocation time.
CONAN_USER_TOOLCHAIN := $(if $(filter cortex-m4,$(FLAVOR)),-c 'tools.cmake.cmaketoolchain:user_toolchain=["$(CURDIR)/cmake/toolchain/cortex-m4-user.cmake"]')

.PHONY: all conan-install sync-presets configure build install compile-commands lockgen-image update-lockfiles clean distclean help

all: build

# Re-run whenever the recipe or either profile changes.
$(BUILD_DIR)/generators/conan_toolchain.cmake: containers/devenv/config/conanfile.py containers/devenv/config/cortex-m4
	conan install containers/devenv/config/conanfile.py -of=. \
		-s build_type=$(BUILD_TYPE) \
		--profile:build=default --profile:host=$(CONAN_HOST_PROFILE) \
		$(CONAN_USER_TOOLCHAIN) \
		--build=missing

conan-install: $(BUILD_DIR)/generators/conan_toolchain.cmake

# CMakeUserPresets.json can only list flavors that have actually been
# installed (cmake errors on a dangling include), and Conan can't manage it
# for us here — conanfile.py's own directory has no CMakeLists.txt, which is
# a precondition for Conan's auto-include. So the Makefile owns it instead:
# drop any entry whose generators file no longer exists (e.g. after
# distclean, which doesn't touch this file), then add the current flavor.
sync-presets: conan-install
	@python3 -c "\
import json, os; \
p = 'CMakeUserPresets.json'; \
entry = 'build/$(BUILD_TYPE)/$(FLAVOR)/generators/CMakePresets.json'; \
data = json.load(open(p)) if os.path.exists(p) else {'version': 4, 'vendor': {'conan': {}}, 'include': []}; \
data['include'] = [e for e in data.get('include', []) if os.path.exists(e)]; \
(entry in data['include']) or data['include'].append(entry); \
json.dump(data, open(p, 'w'), indent=4)"

# Configure only if the build dir doesn't exist yet or CMakeCache is missing
$(BUILD_DIR)/CMakeCache.txt: sync-presets
	cmake --preset $(PRESET) \
		-DCMAKE_INSTALL_PREFIX=$(PREFIX) \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON

configure: $(BUILD_DIR)/CMakeCache.txt

build: configure
	cmake --build --preset $(PRESET) --parallel $(JOBS)

install: build
	cmake --install $(BUILD_DIR)

# Merges every configured flavor's compile_commands.json into one at the repo
# root, for eglot/clangd to see the whole tree. Run after configuring
# whichever flavors you want covered (e.g. make configure; make configure
# FLAVOR=cortex-m4; make compile-commands). Files built under more than one
# flavor (host-portable modules) keep whichever flavor sorts first.
compile-commands:
	@python3 -c "\
import json, glob; \
merged, seen = [], set(); \
files = sorted(glob.glob('build/*/*/compile_commands.json')); \
entries = [e for f in files for e in json.load(open(f))]; \
[(merged.append(e), seen.add(e['file'])) for e in entries if e['file'] not in seen]; \
json.dump(merged, open('compile_commands.json', 'w'), indent=2); \
print(f'{len(merged)} entries from {len(files)} flavor(s) -> compile_commands.json')"

lockgen-image: configure
	docker build --target lockgen -t devenv:lockgen -f $(BUILD_DIR)/config/Dockerfile.devenv $(BUILD_DIR)

update-lockfiles: lockgen-image
	docker run --rm -v "$(CURDIR):/workspace" -w /workspace devenv:lockgen \
	    bash -c "cd containers/devenv/config && uv lock"
	docker run --rm -v "$(CURDIR):/workspace" -w /workspace devenv:lockgen \
	    bash -c " \
	        cd containers/devenv/config && \
	        conan profile detect --force && \
	        cp /root/.conan2-seed-profiles/cortex-m4 /root/.conan2/profiles/cortex-m4 && \
	        conan lock create . --profile:build=default --profile:host=default --lockfile-out=conan.lock && \
	        conan lock create . --profile:build=default --profile:host=cortex-m4 --lockfile=conan.lock --lockfile-out=conan.lock --lockfile-partial \
	    "

clean:
	cmake --build --preset $(PRESET) --target clean

distclean:
	rm -rf build

help:
	@echo "Targets:"
	@echo "  all        - configure + build (default)"
	@echo "  build      - build using BUILD_TYPE=$(BUILD_TYPE) FLAVOR=$(FLAVOR)"
	@echo "  install    - build then install to PREFIX=$(PREFIX)"
	@echo "  clean      - clean object files via CMake"
	@echo "  distclean  - remove the entire build/ directory"
	@echo ""
	@echo "Override with: make BUILD_TYPE=Debug FLAVOR=cortex-m4 PREFIX=/opt/foo"

#___oOo___
