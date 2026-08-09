INCLUDE_DIR := inc
SRC_DIR := src
OUT_DIR := out
OBJ_DIR := $(OUT_DIR)/obj

CC := gcc
CFLAGS := -I$(INCLUDE_DIR) -MMD -MP

SRC := $(wildcard $(SRC_DIR)/*.c)
OBJ := $(patsubst $(SRC_DIR)/%.c, $(OBJ_DIR)/%.o, $(SRC))
DEPS := $(OBJ:.o=.d)

TARGET := $(OUT_DIR)/c2rust_idiomatizer_test

.PHONY: debug run run_debug valgrind clean

####################################################################################################

$(TARGET): $(OBJ)
	mkdir -p $(dir $@)
	$(CC) $^ -o $@ $(LDFLAGS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c Makefile
	mkdir -p $(dir $@)
	$(CC) -c $< -o $@ $(CFLAGS)

-include $(DEPS)

####################################################################################################

run: $(TARGET)
	$(TARGET)

####################################################################################################

clean:
	rm -rf $(OUT_DIR)/*
