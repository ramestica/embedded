REGISTRY  ?= ghcr.io/yourorg
TAG       ?= latest

IMAGES := devenv

.PHONY: all $(IMAGES) push clean

all: $(IMAGES)

$(IMAGES):
	docker build -t $@:$(TAG) $@/

push: all
	$(foreach img,$(IMAGES),docker push $(img):$(TAG);)

clean:
	$(foreach img,$(IMAGES),docker rmi -f $(img):$(TAG);)
