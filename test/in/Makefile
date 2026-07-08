CC = gcc
CFLAGS = -I$(INCLUDE_DIR) -MMD -MP

INCLUDE_DIR = inc
OBJ_DIR = out/obj

SRC = $(wildcard src/*.c)
OBJ = $(patsubst src/%.c, $(OBJ_DIR)/%.o, $(SRC))
DEPS = $(OBJ:.o=.d)

TARGET = out/c2rust_idiomatizer_test

.PHONY: run clean

####################################################################################################

$(TARGET): $(OBJ)
	mkdir -p $(dir $@)
	$(CC) $^ -o $@

-include $(DEPS)

$(OBJ_DIR)/%.o: src/%.c
	mkdir -p $(dir $@)
	$(CC) -c $< -o $@ $(CFLAGS)

####################################################################################################

run: $(TARGET)
	$(TARGET)

####################################################################################################

clean:
	rm -rf out/*