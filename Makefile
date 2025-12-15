CC               = gcc
ENABLE_ASSERT    = -DENABLE_ASSERT=1
CFLAGS           = -std=c99 $(ENABLE_ASSERT) -I./std/ -I./std/mem/ -I./src
CFLAGS  	     += -Wall -Wextra -Wpedantic \
					-Wconversion \
                    -Wformat=2 -Wno-unused-parameter -Wshadow \
                    -Wwrite-strings -Wstrict-prototypes -Wold-style-definition \
                    -Wredundant-decls -Wnested-externs -Wmissing-include-dirs
SANITIZE         = -fsanitize=address,undefined -fno-omit-frame-pointer
TARGET_DIR       = bin
TARGET_NAME 	 = ionc
BUILD_DIR        = build
TARGET           = $(TARGET_DIR)/$(TARGET_NAME)

SRC              = ./src/main.c
ALL_SRC          = $(SRC)

OBJ              = $(patsubst ./src/%.c, $(BUILD_DIR)/%.o, $(SRC))

DEBUGFLAGS       = -DDEBUG -ggdb -O0
RELEASEFLAGS     = -O3

TEST_SUITES := $(notdir $(shell find tests -mindepth 1 -maxdepth 1 -type d))
TEST_BINS   := $(addprefix $(TARGET_DIR)/test-,$(TEST_SUITES))

.PHONY: all clean debug release test-all test-% help clean-tests

all: $(TARGET_DIR) $(TARGET)

$(TARGET_DIR):
	@mkdir -p $(TARGET_DIR)

$(TARGET): $(OBJ)
	@$(CC) -o $@ $(OBJ) $(CFLAGS)

$(BUILD_DIR)/%.o: ./src/%.c
	@mkdir -p $(BUILD_DIR)
	@$(CC) $(CFLAGS) -c $< -o $@

$(TARGET_DIR)/test-%: tests/%-main.c | $(TARGET_DIR)
	@echo "Building single-TU test binary for $*"
	@$(CC) $(CFLAGS) -o $@ $<
	@echo "✓ built test-$*"

test-%: $(TARGET_DIR)/test-%
	@echo "Running tests for $*"
	@./tests/test.sh $*

test-all: $(TEST_SUITES:%=test-%)

run: all
	@$(TARGET)

clean-tests:
	@echo "Cleaning all test outputs"
	@for suite in $(TEST_SUITES); do \
	    rm -f tests/$$suite/*.out tests/$$suite/*.diff; \
	done

clean: clean-tests
	rm -f $(TARGET) $(OBJ)
	rm -rf $(TARGET_DIR)
	rm -rf $(BUILD_DIR)

# ------------ DEBUG / RELEASE ------------
debug: CFLAGS += $(DEBUGFLAGS) $(SANITIZE)
debug: clean all

release: CFLAGS += $(RELEASEFLAGS)
release: clean all


help:
	@echo "Ion Makefile Commands:"
	@echo "  make all          - build main compiler"
	@echo "  make debug        - build main compiler in debug mode"
	@echo "  make release      - build main compiler in release mode"
	@echo "  make test-all     - build all test binaries"
	@echo "  make test-<suite> - build + run tests for suite (lexer, parser, ...)"
	@echo "  make clean        - remove build files"
	@echo "  make clean-tests  - remove .out/.diff files"
