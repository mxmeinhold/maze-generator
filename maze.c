#include "stack.h"
#include "tree.h"
#include "generator.h"
#include <stdio.h>
#include <stdint.h> // intmax_t
#include <img.h>
#include <stdlib.h> // calloc(), srand(), atexit()
#include <string.h> // strcmp()
#include <time.h>   // time()

#include "format.h" // ANSI formatting escape sequences

#define STRINGIFY2(X) #X
#define STRINGIFY(X) STRINGIFY2(X)

// Size used if not specified by flags
#define DEFAULT_SIZE 50
// Default output path
#define DEFAULT_OUTFILE "maze.png"
#define DEFAULT_SEED time(0)

/** The usage message */
char* usage;

/** Arguments for the usage message */
static const char* args_doc =
    "[-h] [--size size] [--rows num_rows] [--cols num_cols] [--seed seed] [-f output_path]";

/** Help message, much more detailed than usage, and with pretty formatting */
static const char* help = BOLD "Options" INTENSITY_RESET "\n"
TAB BOLD"-h"INTENSITY_RESET":\n"TAB TAB"print this help message\n"
TAB BOLD"--size"INTENSITY_RESET" "UNDERLINE"size"UNDERLINE_OFF":\n"TAB TAB"create a maze that is "UNDERLINE"size"UNDERLINE_OFF" rows by "UNDERLINE"size"UNDERLINE_OFF" columns (overridden by --rows and --cols). default: "STRINGIFY(DEFAULT_SIZE)"\n"
TAB BOLD"--rows"INTENSITY_RESET" "UNDERLINE"num_rows"UNDERLINE_OFF":\n"TAB TAB"sets the maze size to "UNDERLINE"num_rows"UNDERLINE_OFF" rows\n"
TAB BOLD"--cols"INTENSITY_RESET" "UNDERLINE"num_cols"UNDERLINE_OFF":\n"TAB TAB"sets the maze size to "UNDERLINE"num_cols"UNDERLINE_OFF" columns\n"
TAB BOLD"--seed"INTENSITY_RESET" "UNDERLINE"seed"UNDERLINE_OFF":\n"TAB TAB"specify a seed for the random number generator\n"
TAB BOLD"-f"INTENSITY_RESET" "UNDERLINE"output_path"UNDERLINE_OFF":\n"TAB TAB"where to write the maze png to. default: "STRINGIFY(DEFAULT_OUTFILE)"\n";

/** Arguments struct  */
struct arguments {
    /** Number of rows in the output maze */
    unsigned long rows;
    /** Number of columns in the output maze */
    unsigned long cols;
    /** Seed to control rng */
    unsigned int seed;
    /** Out file name */
    const char* out_file;
    /** Exit immediately flag */
    volatile short exit;
};

/**
 * Hash a string to an int.
 *
 * Uses a slight modification of `djb2` (credit to
 * http://www.cse.yorku.ca/~oz/hash.html)
 */
unsigned int hash_string(unsigned const char* str) {
    unsigned long hash = 5381;
    unsigned char c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) ^ c;
    }

    unsigned int out = 0;
    for (size_t i = 0; i < sizeof(hash) / sizeof(out); i++) {
        out ^= (unsigned int) hash;
        hash = hash >> (sizeof(out) * 8);
    }

    return out;
}

void relocate(struct cell* c) {
    c->row = c->row * 2 + 1;
    c->col = c->col * 2 + 1;
}

intmax_t strcompare(const void* self, const void* other) {
    return (intmax_t) strcmp((void*) self, (void*) other);
}

/**
 * get_usage(): Allocates a usage message string
 *
 * Arguments:
 * - argc: argc as passed to main()
 * - argv: argv as passed to main()
 *
 * Return:
 *   A pointer to the usage message
 *
 * Note:
 *   The usage message is allocated dynamically, and must be freed by the
 *   caller
 */
static char* get_usage(const int argc, const char** argv) {
    char* usage_msg = malloc(10 + strlen(argv[0]) + strlen(args_doc));
    sprintf(usage_msg, "Usage: %s %s\n", argv[0], args_doc);
    return usage_msg;
}

/**
 * parse_args: Parses argc and argv into the structure at args
 *
 * Arguments:
 * - args_p: Pointer to struct arguments where parsed values should be stored
 * - argc: length of argv
 * - argv: standard argv array as per c99
 *
 * Return:
 *   Upon a successful call, parse_args() returns 0. Otherwise, an error code
 *   is returned.
 *
 * Errors:
 * - -1: An error specific to this function occurred
 * -  1: An error in resolving the destination occurred
 * -  2: Arguments are bad values and unparseable
 *
 * Note:
 *   If the -h flag is invoked, parse_args() will print the usage and help
 *   strings and _exit the program_.
 */
static int parse_args(
        struct arguments* args_p,
        const int argc,
        const char** argv
    ) {

    // Setup some defaults
    args_p->out_file = DEFAULT_OUTFILE;
    args_p->seed = (unsigned int) DEFAULT_SEED;

    // If any arg is -h, print help and exit
    if (argc  >= 2) {
        for (int i = 0; i < argc; i++) {
            if (strncmp("-h", argv[i], 2) == 0) {
                printf("%s\n%s", usage,  help);
                exit(0);
            }
        }
    }


    // Parse the other flags, if there are any
    char* endptr;
    unsigned long size = DEFAULT_SIZE, rows = 0, cols = 0;
    if (argc > 2) {
        for (int i = 1; i < argc; i++) {
            if (i == argc - 1) {
                fprintf(stderr, "Error: Missing argument for %s.\n", argv[i]);
                fprintf(stderr, "%s\n", usage);
                return 2; // User gave bad input
            } else if (strncmp(argv[i], "--size", 6) == 0) {
                size = strtoul(argv[++i], &endptr, 10);
                if (size < 1 || *endptr != '\0') {
                    fprintf(stderr, "Error: `%s` is not a valid argument for "
                            "--size (must be a positive integer)\n", argv[i]);
                    return 2; // User gave bad values
                }
            } else if (strncmp(argv[i], "--rows", 6) == 0) {
                rows = strtoul(argv[++i], &endptr, 10);
                if (rows < 1 || *endptr != '\0') {
                    fprintf(stderr, "Error: `%s` is not a valid argument for "
                            "--rows (must be a positive integer)\n", argv[i]);
                    return 2; // User gave bad values
                }
            } else if (strncmp(argv[i], "--cols", 6) == 0) {
                cols = strtoul(argv[++i], &endptr, 10);
                if (cols < 1 || *endptr != '\0') {
                    fprintf(stderr, "Error: `%s` is not a valid argument for "
                            "--cols (must be a positive integer)\n", argv[i]);
                    return 2; // User gave bad values
                }
            } else if (strncmp(argv[i], "-f", 2) == 0) {
                args_p->out_file = argv[++i];
            } else if (strncmp(argv[i], "--seed", 6) == 0) {
               args_p->seed = hash_string((unsigned const char*)argv[++i]);
            }
        }
    }

    // Default rows and cols to size
    if (rows == 0) rows = size;
    if (cols == 0) cols = size;
    args_p->rows = rows;
    args_p->cols = cols;

    return 0;
}

/** cleanup actions to take on normal exit */
void cleanup(void) {
    free(usage);
}

int main(const int argc, const char** argv) {
    atexit(cleanup);

    usage = get_usage(argc, argv);

    struct arguments args;
    parse_args(&args, argc, argv);
    srand(args.seed);

    struct maze* maze = gen_maze(args.rows, args.cols, &relocate);

    struct img img;
    img.width = (int) (2 * args.rows + 1);
    img.height = (int) (2 * args.cols + 1);
    img.rows = calloc(sizeof(struct pixel*), (size_t)img.height);
    for (int r = 0; r < img.height; r++) {
        img.rows[r] = calloc(sizeof(struct pixel), (size_t)img.width);
        for (int c = 0; c < img.width; c++) {
            // It might be a wall. More on that later.
            img.rows[r][c].red = 0;
            img.rows[r][c].green = 0;
            img.rows[r][c].blue = 0;
        }
    }

    for (unsigned short r = 0; r < maze->rows; r++) {
        for (unsigned short c = 0; c < maze->cols; c++) {
            struct cell* cell = maze->maze[r][c];
            img.rows[cell->row][cell->col].red = 255;
            img.rows[cell->row][cell->col].green = 255;
            img.rows[cell->row][cell->col].blue = 255;
            for (struct list_node* path = cell->paths.start; path != NULL; path = path->next) {
                long row = (long) path->cell->row + ((long)cell->row - (long)path->cell->row) / 2;
                long col = (long) path->cell->col + ((long)cell->col - (long)path->cell->col) / 2;
                img.rows[row][col].red = 255;
                img.rows[row][col].green = 255;
                img.rows[row][col].blue = 255;
            }
        }
    }
    writepng(args.out_file, &img);
    clean_maze(maze);

    // Free the image
    for (int r = 0; r < img.height; r++) free(img.rows[r]);
    free(img.rows);

    return 0;
}
