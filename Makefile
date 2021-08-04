SHELL := bash
.ONESHELL:
.SHELLFLAGS := -eu -o pipefail -c
.DELETE_ON_ERROR:
MAKEFLAGS += --warn-undefined-variables --no-builtin-rules

CC = gcc
CFLAGS += -std=c99
LDFLAGS ?=

# If DEBUG is set, compile with gdb flags
ifdef DEBUG
CFLAGS += -g -ggdb
endif

# Warnings
WARNINGS = -Wall -Wextra -Wpedantic -Wconversion -Wformat=2 -Winit-self \
	-Wmissing-include-dirs -Wformat-nonliteral -Wnested-externs \
	-Wno-unused-parameter -Wold-style-definition -Wredundant-decls -Wshadow \
	-Wstrict-prototypes -Wwrite-strings

# GCC warnings that Clang doesn't provide:
ifeq ($(CC),gcc)
		WARNINGS += -Wjump-misses-init -Wlogical-op
endif

BUILD_DIR = target

ANALYSIS_DIR = $(BUILD_DIR)/analysis

SOURCEDIR = .
SOURCES := $(subst ./,,$(shell find $(SOURCEDIR) -name '*.c'))
_O_FILES = $(SOURCES:%.c=%.o )
O_DIR = $(BUILD_DIR)/obj
O_FILES = $(patsubst %,$(O_DIR)/%,$(_O_FILES))

D_DIR = $(BUILD_DIR)/d
D_FILES = $(patsubst %.o,$(D_DIR)/%.d,$(_O_FILES))
D_FILES += $(D_DIR)/main

MAIN = test.c
EXEC = $(BUILD_DIR)/test

LIBRARIES = -limg -lpng

default: $(EXEC)

$(EXEC): $(O_FILES)
	$(CC) -fPIC -o $(EXEC) $^ $(CFLAGS) $(WARNINGS) $(LIBRARIES)

-include $(D_FILES)

# Use gcc to identify dependencies of each source
# TODO this doesn't handle the EXEC
$(D_DIR)/%.d: %.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) $< -MM -MT $(O_DIR)/$(<:.c=.o) > $@
	echo -e '\tmkdir -p $$(@D)' >> $@
	echo -e '\t$$(CC) -fPIC -c -o $$@ $$< $$(CFLAGS) $$(LDFLAGS) $$(WARNINGS)' >> $@

.PHONY: run
run: $(EXEC)
	./$(EXEC)

VALGRIND_DEP = $(BUILD_DIR)/.valgrind
$(VALGRIND_DEP):
	@command -v valgrind >/dev/null 2>&1 || { echo >&2 "valgrind not found, aborting analyze"; exit 1; }
	@touch $@

INOTIFY_DEP = $(BUILD_DIR)/.inotify
$(INOTIFY_DEP):
	@command -v inotifywait >/dev/null 2>&1 || { echo >&2 "inotifywait not found, aborting watch"; exit 1; }
	@touch $@

.PHONY: analyze
analyze: $(ANALYSIS_DIR)/memcheck.out $(ANALYSIS_DIR)/callgrind.out $(ANALYSIS_DIR)/callgrind.png

$(ANALYSIS_DIR)/memcheck.out: $(EXEC) $(VALGRIND_DEP)
	@mkdir -p $(@D)
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes --log-file="$@" ./$(EXEC)

$(ANALYSIS_DIR)/callgrind.out: $(EXEC) $(VALGRIND_DEP)
	@mkdir -p $(@D)
	valgrind --tool=callgrind --callgrind-out-file="$@" ./$(EXEC)

$(ANALYSIS_DIR)/callgrind.png: $(ANALYSIS_DIR)/callgrind.out
	gprof2dot -f callgrind $< --root=main | dot -Tpng -o $@


.PHONY: watch
watch: run
	while true; do \
		clear; \
		make run || true; \
		inotifywait -qre modify .; \
	done

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
