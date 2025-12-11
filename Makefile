CC               = gcc
ENABLE_ASSERT    = -DENABLE_ASSERT=1
CFLAGS           = -std=c99 $(ENABLE_ASSERT) -I./std/ -I./std/mem/
CFLAGS  	     += -Wall -Wextra -Wpedantic \
					-Wconversion \
                    -Wformat=2 -Wno-unused-parameter -Wshadow \
                    -Wwrite-strings -Wstrict-prototypes -Wold-style-definition \
                    -Wredundant-decls -Wnested-externs -Wmissing-include-dirs
SANITIZE         = -fsanitize=address,undefined -fno-omit-frame-pointer
TARGET_DIR       = bin
BUILD_DIR        = build
TARGET           = $(TARGET_DIR)/main

SRC              = ./src/main.c
ALL_SRC          = $(SRC)

OBJ              = $(patsubst ./src/%.c, $(BUILD_DIR)/%.o, $(SRC))

DEBUGFLAGS       = -DDEBUG -ggdb -O0
RELEASEFLAGS     = -O2

.PHONY: all clean run debug release

# ----------- NORMAL BUILD -----------
all: $(TARGET_DIR) $(TARGET)

$(TARGET_DIR):
	@mkdir -p $(TARGET_DIR)

$(TARGET): $(OBJ)
	@$(CC) -o $@ $(OBJ) $(CFLAGS)

$(BUILD_DIR)/%.o: ./src/%.c
	@mkdir -p $(BUILD_DIR)
	@$(CC) $(CFLAGS) -c $< -o $@

run: all
	@$(TARGET)

clean:
	rm -f $(TARGET) $(OBJ)
	rm -rf $(TARGET_DIR)
	rm -rf $(BUILD_DIR)

# ------------ DEBUG / RELEASE ------------
debug: CFLAGS += $(DEBUGFLAGS) $(SANITIZE)
debug: clean all

release: CFLAGS += $(RELEASEFLAGS)
release: clean all
