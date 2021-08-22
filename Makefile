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

ifdef STATIC
CFLAGS += -static
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
_O_FILES = $(SOURCES:%.c=%.o)
O_DIR = $(BUILD_DIR)/obj

D_DIR = $(BUILD_DIR)/d
D_FILES = $(patsubst %.o,$(D_DIR)/%.d,$(_O_FILES))

LIBRARIES = -limg -lpng16 -lz -lm

MAZE_EXEC = $(BUILD_DIR)/maze
TXT_TO_PNG_EXEC = $(BUILD_DIR)/txt-to-png

EXECS = $(MAZE_EXEC) $(TXT_TO_PNG_EXEC)

default: $(EXECS)

MAZE_O_FILES = maze.o tree.o dfs.o stack.o linked_list.o
$(MAZE_EXEC): $(patsubst %.o,$(O_DIR)/%.o,$(MAZE_O_FILES))
	$(CC) -fPIC -o $@ $^ $(CFLAGS) $(WARNINGS) $(LIBRARIES)

TXT_TO_PNG_O_FILES = txt-to-png.o
$(TXT_TO_PNG_EXEC): $(patsubst %.o,$(O_DIR)/%.o,$(TXT_TO_PNG_O_FILES))
	$(CC) -fPIC -o $@ $^ $(CFLAGS) $(WARNINGS) $(LIBRARIES)


-include $(D_FILES)

# Use gcc to identify dependencies of each source
$(D_DIR)/%.d: %.c
	mkdir -p $(@D)
	$(CC) $(CFLAGS) $< -MM -MT $(O_DIR)/$(<:.c=.o) > $@
	echo -e '\tmkdir -p $$(@D)' >> $@
	echo -e '\t$$(CC) -fPIC -c -o $$@ $$< $$(CFLAGS) $$(LDFLAGS) $$(WARNINGS)' >> $@

.PHONY: run
run: $(MAZE_EXEC)
	./$(MAZE_EXEC)

VALGRIND_DEP = $(BUILD_DIR)/.valgrind
$(VALGRIND_DEP):
	@command -v valgrind >/dev/null 2>&1 || { echo >&2 "valgrind not found, aborting analyze"; exit 1; }
	@touch $@

INOTIFY_DEP = $(BUILD_DIR)/.inotify
$(INOTIFY_DEP):
	@command -v inotifywait >/dev/null 2>&1 || { echo >&2 "inotifywait not found, aborting watch"; exit 1; }
	@touch $@

# Rule for generating mazes
MAZE_DIR = $(BUILD_DIR)/mazes
$(MAZE_DIR)/maze.%: $(MAZE_EXEC)
	@mkdir -p $(@D)
	./$< --format $* -f $@


# Analysis
.PHONY: analyze
analyze: maze.analyze txt-to-png.analyze

maze.analyze txt-to-png.analyze: %.analyze: $(ANALYSIS_DIR)/memcheck.%.out $(ANALYSIS_DIR)/callgrind.%.out $(ANALYSIS_DIR)/callgrind.%.png

$(ANALYSIS_DIR)/maze.test_args:
	@mkdir -p $(@D)
	@mkdir -p $(MAZE_DIR)
	echo "--format png --size 50 -f $(MAZE_DIR)/maze.maze.png" > $@

$(ANALYSIS_DIR)/txt-to-png.test_args: $(MAZE_DIR)/maze.text
	@mkdir -p $(@D)
	echo "$(MAZE_DIR)/maze.text $(MAZE_DIR)/maze.txt-to-png.png" > $@

$(ANALYSIS_DIR)/%.test_args:
	@mkdir -p $(@D)
	touch $@


$(ANALYSIS_DIR)/memcheck.%.out: $(BUILD_DIR)/% $(VALGRIND_DEP) $(ANALYSIS_DIR)/%.test_args
	@mkdir -p $(@D)
	valgrind --tool=memcheck --leak-check=full --show-leak-kinds=all --track-origins=yes --log-file="$@" ./$< $$(cat $(ANALYSIS_DIR)/$*.test_args)


$(ANALYSIS_DIR)/callgrind.%.out: $(BUILD_DIR)/% $(VALGRIND_DEP) $(ANALYSIS_DIR)/%.test_args
	@mkdir -p $(@D)
	valgrind --tool=callgrind --callgrind-out-file="$@" ./$< $$(cat $(ANALYSIS_DIR)/$*.test_args)


$(ANALYSIS_DIR)/callgrind.%.png: $(ANALYSIS_DIR)/callgrind.%.out
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
