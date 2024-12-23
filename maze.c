#include "stack.h"
#include "tree.h"
#include "generator.h"
#include <stdio.h>
#include <stdint.h> // intmax_t
#include <img.h>
#include <stdlib.h> // calloc(), srand(), atexit()
#include <string.h> // strcmp(), strstr()
#include <time.h>   // time()

#include "format.h" // ANSI formatting escape sequences

#define STRINGIFY2(X) #X
#define STRINGIFY(X) STRINGIFY2(X)

// Size used if not specified by flags
#define DEFAULT_SIZE 50
// Default output path and format
#define DEFAULT_OUTFILE "maze.png"
#define DEFAULT_OUT_FORMAT "png"
#define VALID_OUT_FORMATS "{png|text}"

#define DEFAULT_SEED time(0)

/** The usage message */
char* usage;

/** Arguments for the usage message */
static const char* args_doc =
    "[-h] [--size size] [--rows num_rows] [--cols num_cols] [--seed seed] [--path-len length] [-f output_path] [--format "VALID_OUT_FORMATS"]";

/** Help message, much more detailed than usage, and with pretty formatting */
static const char* help = BOLD "Options" INTENSITY_RESET "\n"
TAB BOLD"-h"INTENSITY_RESET":\n"TAB TAB"print this help message\n"
TAB BOLD"--size"INTENSITY_RESET" "UNDERLINE"size"UNDERLINE_OFF":\n"TAB TAB"create a maze that is "UNDERLINE"size"UNDERLINE_OFF" rows by "UNDERLINE"size"UNDERLINE_OFF" columns (overridden by --rows and --cols). default: "STRINGIFY(DEFAULT_SIZE)"\n"
TAB BOLD"--rows"INTENSITY_RESET" "UNDERLINE"num_rows"UNDERLINE_OFF":\n"TAB TAB"sets the maze size to "UNDERLINE"num_rows"UNDERLINE_OFF" rows\n"
TAB BOLD"--cols"INTENSITY_RESET" "UNDERLINE"num_cols"UNDERLINE_OFF":\n"TAB TAB"sets the maze size to "UNDERLINE"num_cols"UNDERLINE_OFF" columns\n"
TAB BOLD"--seed"INTENSITY_RESET" "UNDERLINE"seed"UNDERLINE_OFF":\n"TAB TAB"specify a seed for the random number generator\n"
TAB BOLD"--path-len"INTENSITY_RESET" "UNDERLINE"length"UNDERLINE_OFF":\n"TAB TAB"limit the length of the path.  default: no limit (0)\n"
TAB BOLD"-f"INTENSITY_RESET" "UNDERLINE"output_path"UNDERLINE_OFF":\n"TAB TAB"where to write the maze png to. default: "STRINGIFY(DEFAULT_OUTFILE)"\n"
TAB BOLD"--format"INTENSITY_RESET" "UNDERLINE""VALID_OUT_FORMATS""UNDERLINE_OFF":\n"TAB TAB"what format to use when writing to the output default: "STRINGIFY(DEFAULT_OUT_FORMAT)"\n"
TAB BOLD"--print-valid-formats"INTENSITY_RESET":\n"TAB TAB"print the valid format strings, one per line, and exit\n"
TAB BOLD"--write-steps"INTENSITY_RESET" "UNDERLINE"prefix"UNDERLINE_OFF":\n"TAB TAB"write each step of maze generation as '"UNDERLINE"prefix"UNDERLINE_OFF"<number>.png'\n";

/** Arguments struct  */
struct arguments {
    /** Number of rows in the output maze */
    unsigned long rows;
    /** Number of columns in the output maze */
    unsigned long cols;
    /** Seed to control rng */
    unsigned int seed;
    /** Path length limit */
    unsigned long limit;
    /** Out file name */
    const char* out_file;
    /** Out format */
    const char* out_format;
    /** Exit immediately flag */
    //volatile short exit; // TODO
};

/**
 * If this isn't NULL, we'll write each step of the maze generation as a
 * separate png. This allows for visual examination of the process, or for
 * making a gif.
 *
 * this is a global because I need to access it in the write_steps function
 */
volatile char* write_steps_prefix;

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

//void relocate(struct cell* c) {
    //c->row = c->row * 2 + 1;
    //c->col = c->col * 2 + 1;
    //return; // TODO
//}

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
    args_p->out_format = DEFAULT_OUT_FORMAT;
    args_p->seed = (unsigned int) DEFAULT_SEED;
    args_p->limit = 0; // No limit
    write_steps_prefix = NULL;

    // If any arg is -h, print help and exit
    if (argc  >= 2) {
        for (int i = 0; i < argc; i++) {
            if (strncmp("-h", argv[i], 2) == 0) {
                printf("%s\n%s", usage,  help);
                exit(0);
            } else if (strncmp("--print-valid-formats", argv[i], 21) == 0) {
                printf("png\n");
                printf("text\n");
                exit(0);
            }
        }
    }


    // Parse the other flags, if there are any
    char* endptr;
    unsigned long size = DEFAULT_SIZE, rows = 0, cols = 0;
    if (argc > 1) {
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
            } else if (strncmp(argv[i], "--path-len", 10) == 0) {
                args_p->limit = strtoul(argv[++i], &endptr, 10);
                if (*endptr != '\0') {
                    fprintf(stderr, "Error: `%s` is not a valid argument for "
                            "--limit (must be a positive integer or 0)\n", argv[i]);
                    return 2; // User gave bad values
                }
            } else if (strncmp(argv[i], "--format", 9) == 0) {
               args_p->out_format = argv[++i];
               // Verify it's valid
               if (strstr(VALID_OUT_FORMATS, args_p->out_format) == NULL) {
                    fprintf(stderr, "Error: `%s` isn't a valid output format.\n",
                            args_p->out_format);
               }
            } else if (strncmp(argv[i], "--write-steps", 15) == 0) {
               write_steps_prefix = (char*) argv[++i];
            } else {
                fprintf(stderr, "Error: `%s` isn't a flag.\n", argv[i]);
                return 2; // User gave bad values
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

// Image for printing steps. Shared by all step prints to reduce allocations
struct img* step_img;

/** cleanup actions to take on normal exit */
void cleanup(void) {
    free(usage);

    if (step_img) {
        for (int r = 0; r < step_img->height; r++) free(step_img->rows[r]);
        free(step_img->rows);
        free(step_img);
    }
}

/** write the maze as a png */
void write_maze_png(const struct maze* maze, const struct cell* current, const char* filename) {
    struct img img;
    img.height = (int) (2 * maze->dims_array[0] + 1);
    img.width = (int) (2 * maze->dims_array[1] + 1);
    img.rows = calloc((size_t)img.height, sizeof(struct pixel*));
    for (int r = 0; r < img.height; r++) {
        img.rows[r] = calloc((size_t)img.width, sizeof(struct pixel));
    }

    unsigned long current_row;
    unsigned long current_col;
    if (current) {
        current_row = current->coords[0] * 2 + 1;
        current_col = current->coords[1] * 2 + 1;
    }

    for (unsigned short r = 0; r < maze->dims_array[0]; r++) {
        for (unsigned short c = 0; c < maze->dims_array[1]; c++) {
            struct cell* cell = maze->maze[r][c];
            unsigned long cell_row = cell->coords[0] * 2 + 1;
            unsigned long cell_col = cell->coords[1] * 2 + 1;
            if (!cell->visited) continue;
            img.rows[cell_row][cell_col].red = 255;
            img.rows[cell_row][cell_col].green = 255;
            img.rows[cell_row][cell_col].blue = 255;
            if (current && (2 * r + 1) == (long)current_row && (2 * c + 1) == (long)current_col) {
                img.rows[cell_row][cell_col].red = 255;
                img.rows[cell_row][cell_col].green = 0;
                img.rows[cell_row][cell_col].blue = 0;
            }
            for (struct list_node* path = cell->paths.start; path != NULL; path = path->next) {
                unsigned long path_row = path->cell->coords[0] * 2 + 1;
                unsigned long path_col = path->cell->coords[1] * 2 + 1;
                long row = (long) path_row + ((long)cell_row - (long)path_row) / 2;
                long col = (long) path_col + ((long)cell_col - (long)path_col) / 2;
                img.rows[row][col].red = 255;
                img.rows[row][col].green = 255;
                img.rows[row][col].blue = 255;
            }
        }
    }
    writepng(filename, &img);

    // Free the image
    for (int r = 0; r < img.height; r++) free(img.rows[r]);
    free(img.rows);
}

/** write the maze as a plaintext file, using ' ' for paths and '#' for walls */
void write_maze_text(const struct maze* maze, struct arguments* args) {
    FILE* file = fopen(args->out_file, "w+");

    int height = (int) (2 * args->rows + 1);
    int width = (int) (2 * args->cols + 1);
    char** rows = calloc((size_t)height, sizeof(char*));
    for (int r = 0; r < height; r++) {
        rows[r] = calloc((size_t)width, sizeof(char));
        for (int c = 0; c < width; c++) {
            // It might be a wall. More on that later.
            rows[r][c] = '#';
        }
    }

    for (unsigned short r = 0; r < maze->dims_array[0]; r++) {
        for (unsigned short c = 0; c < maze->dims_array[1]; c++) {
            struct cell* cell = maze->maze[r][c];
            unsigned long cell_row = cell->coords[0] * 2 + 1;
            unsigned long cell_col = cell->coords[1] * 2 + 1;
            rows[cell_row][cell_col] = ' ';
            for (struct list_node* path = cell->paths.start; path != NULL; path = path->next) {
                unsigned long path_row = path->cell->coords[0] * 2 + 1;
                unsigned long path_col = path->cell->coords[1] * 2 + 1;
                long row = (long) path_row + ((long)cell_row - (long)path_row) / 2;
                long col = (long) path_col + ((long)cell_col - (long)path_col) / 2;
                rows[row][col] = ' ';
            }
        }
    }

    // Actually write out the maze
    for (int r = 0; r < height; r++) {
        for (int c = 0; c < width; c++) {
            fputc(rows[r][c], file);
        }
        fputc('\n', file);
    }

    // Free the buffer
    for (int r = 0; r < height; r++) free(rows[r]);
    free(rows);
}

void write_step(const struct maze* maze, const struct cell* current, const unsigned int step) {
    unsigned long current_row;
    unsigned long current_col;
    if (current) {
        current_row = current->coords[0] * 2 + 1;
        current_col = current->coords[1] * 2 + 1;
    }

    // Init our cache on the first run
    if (!step_img) {
        step_img = calloc(1, sizeof(struct img));
        step_img->height = (int) (2 * maze->dims_array[0] + 1);
        step_img->width = (int) (2 * maze->dims_array[1] + 1);
        step_img->rows = calloc((size_t)step_img->height, sizeof(struct pixel*));
        for (int r = 0; r < step_img->height; r++) {
            step_img->rows[r] = calloc((size_t)step_img->width, sizeof(struct pixel));
        }

        // On the first run, generate the whole image
        for (unsigned short r = 0; r < maze->dims_array[0]; r++) {
            for (unsigned short c = 0; c < maze->dims_array[1]; c++) {
                struct cell* cell = maze->maze[r][c];
                if (!cell->visited) continue;
                unsigned long cell_row = cell->coords[0] * 2 + 1;
                unsigned long cell_col = cell->coords[1] * 2 + 1;
                step_img->rows[cell_row][cell_col].red = 255;
                step_img->rows[cell_row][cell_col].green = 255;
                step_img->rows[cell_row][cell_col].blue = 255;
                if (current && (2 * r + 1) == (long)current_row && (2 * c + 1) == (long)current_col) {
                    step_img->rows[cell_row][cell_col].red = 255;
                    step_img->rows[cell_row][cell_col].green = 0;
                    step_img->rows[cell_row][cell_col].blue = 0;
                }
                for (struct list_node* path = cell->paths.start; path != NULL; path = path->next) {
                    unsigned long path_row = path->cell->coords[0] * 2 + 1;
                    unsigned long path_col = path->cell->coords[1] * 2 + 1;
                    long row = (long) path_row + ((long)cell_row - (long)path_row) / 2;
                    long col = (long) path_col + ((long)cell_col - (long)path_col) / 2;
                    step_img->rows[row][col].red = 255;
                    step_img->rows[row][col].green = 255;
                    step_img->rows[row][col].blue = 255;
                }
            }
        }
    } else {
        if (!current) return;
        unsigned long rows = maze->dims_array[0];
        unsigned long cols = maze->dims_array[1];

        unsigned short rmin = (unsigned short)(current_row + (current_row > 0)? -1 : 0);
        unsigned short cmin = (unsigned short)(current_col + (current_col > 0)? -1 : 0);
        unsigned short rmax = (unsigned short)(current_row + (current_row + 1 < rows)? 1 : rows - 1);
        unsigned short cmax = (unsigned short)(current_col + (current_col + 1 < cols)? 1 : cols - 1);

        // We only have to update anything immediately surrounding the current
        for (unsigned short r = rmin; r <= rmax; r++) {
            for (unsigned short c = cmin; c <= cmax; c++) {
                struct cell* cell = maze->maze[r][c];
                unsigned long cell_row = cell->coords[0] * 2 + 1;
                unsigned long cell_col = cell->coords[1] * 2 + 1;
                if (!cell->visited) continue;
                step_img->rows[cell_row][cell_col].red = 255;
                step_img->rows[cell_row][cell_col].green = 255;
                step_img->rows[cell_row][cell_col].blue = 255;
                if (current && (2 * r + 1) == (long)current_row && (2 * c + 1) == (long)current_col) {
                    step_img->rows[cell_row][cell_col].red = 255;
                    step_img->rows[cell_row][cell_col].green = 0;
                    step_img->rows[cell_row][cell_col].blue = 0;
                }
                for (struct list_node* path = cell->paths.start; path != NULL; path = path->next) {
                    unsigned long path_row = path->cell->coords[0] * 2 + 1;
                    unsigned long path_col = path->cell->coords[1] * 2 + 1;
                    long row = (long) path_row + ((long)cell_row - (long)path_row) / 2;
                    long col = (long) path_col + ((long)cell_col - (long)path_col) / 2;
                    step_img->rows[row][col].red = 255;
                    step_img->rows[row][col].green = 255;
                    step_img->rows[row][col].blue = 255;
                }
            }
        }
    }
    char out_file[40];
    sprintf(out_file, "%s%04d.png", write_steps_prefix, step);
    writepng(out_file, step_img);
}

int main(const int argc, const char** argv) {
    atexit(cleanup);

    usage = get_usage(argc, argv);

    struct arguments args;
    int err = parse_args(&args, argc, argv);
    if (err) return err;
    srand(args.seed);

    struct maze* maze;
    if (write_steps_prefix == NULL) {
        maze = gen_maze_4(args.rows, args.cols, args.limit, NULL);
    } else {
        maze = gen_maze_4(args.rows, args.cols, args.limit, &write_step);
    }

    if (strcmp("png", args.out_format) == 0)
        write_maze_png(maze, NULL, args.out_file);
    else if (strcmp("text", args.out_format) == 0)
        write_maze_text(maze, &args);

    clean_maze(maze);

    return 0;
}
