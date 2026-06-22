# myproject/Makefile
BUILD_TYPE  ?= Release
BUILD_DIR   := build/$(BUILD_TYPE)
DEV_IMAGE   ?= devenv:latest

# ... normal targets ...

# Drop into a dev shell inside the container
shell:
	docker run --rm -it \
	    -v $(PWD):/workspace \
	    -w /workspace \
	    $(DEV_IMAGE)

# Run the full build inside the container
docker-build:
	docker run --rm \
	    -v $(PWD):/workspace \
	    -w /workspace \
	    $(DEV_IMAGE) \
	    make BUILD_TYPE=$(BUILD_TYPE)
