CC := gcc
#CFLAGS := -Wall -Wextra -Werror -O2 -MMD -MP
CFLAGS := -std=c23 -Wall -Wextra -O2 -MMD -MP
SRCDIR := src
BUILDDIR := build
TARGET := $(BUILDDIR)/main

SRC := $(wildcard $(SRCDIR)/*.c)
OBJ := $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(SRC))
DEPS := $(OBJ:.o=.d)

.PHONY: all build run clean

all: build

build: $(TARGET)

# Link
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Compile into build/
$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	@mkdir -p $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Include auto-generated dependency files (.d)
-include $(DEPS)

run: build
	./$(TARGET)

clean:
	rm -rf $(BUILDDIR)