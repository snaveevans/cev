FRAMEWORKS     = -framework Carbon
BUILD_PATH     = ./bin
BUILD_FLAGS    = -Wall -g
CEV_SRC        = ./src/cev.c
BINS           = $(BUILD_PATH)/cev

all: $(BINS)

.PHONY: all clean install

install: BUILD_FLAGS=-O2
install: clean $(BINS)

$(BINS): | $(BUILD_PATH)

$(BUILD_PATH):
	mkdir -p $(BUILD_PATH)

clean:
	rm -rf $(BUILD_PATH)

$(BUILD_PATH)/cev: $(CEV_SRC)
	clang $^ $(BUILD_FLAGS) $(FRAMEWORKS) -o $@
