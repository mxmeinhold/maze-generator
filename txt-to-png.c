#include <img.h>
#include <stdlib.h> // free(), calloc()

#define WALL '#'
#define SPACE ' '
#define PATH '*'

// argv[0]: input text file
// argv[1]: output png file
int main(int argc, char** argv) {
    FILE* file = fopen(argv[1], "r");
    int rows = 0;
    int cols = 0;

    int ch;

    while ((ch = fgetc(file)) != EOF) {
        if (ch == '\n') {
            rows++;
        }
        if (rows == 0) {
            cols++;
        }
    }

    fseek(file, 0, SEEK_SET);
    
    struct img img;
    img.height = rows;
    img.width = cols;
    img.rows = calloc(sizeof(struct pixel*), (size_t)img.height);
    for (int r = 0; r < img.height; r++) {
        img.rows[r] = calloc(sizeof(struct pixel), (size_t)img.width);
        for (int c = 0; c < img.width; c++) {
            ch = fgetc(file);

            // Skip newlines
            if (ch == '\n') {
                c--;
                continue;
            }

            // It might be a wall. More on that later.
            if (ch == WALL) {
                img.rows[r][c].red = 0;
                img.rows[r][c].green = 0;
                img.rows[r][c].blue = 0;
            } else if (ch == PATH) {
                img.rows[r][c].red = 255;
                img.rows[r][c].green = 0;
                img.rows[r][c].blue = 0;
            } else if (ch == SPACE) {
                img.rows[r][c].red = 255;
                img.rows[r][c].green = 255;
                img.rows[r][c].blue = 255;
            }
        }
    }

    writepng(argv[2], &img);

    // Free the image
    for (int r = 0; r < img.height; r++) free(img.rows[r]);
    free(img.rows);
    fclose(file);
}
