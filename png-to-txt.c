#include <img.h>
#include <stdlib.h> // free(), calloc()
#include <stdio.h> // file handling

#include "text-format.h"

// argv[0]: input png file
// argv[1]: output text file
int main(int argc, char** argv) {
    struct img img;

    // TODO print an actual error message
    int status = readpng(argv[1], &img);
    if (status != 0) {
        return status;
    }
    
    FILE* file = fopen(argv[2], "w");

    for (int row = 0; row < img.height; row++) {
        for (int col = 0; col < img.width; col++) {
            struct pixel p = img.rows[row][col];

            if (p.red == 255 && p.green == 255 && p.blue == 255) {
                fputc(SPACE, file);
            } else if (p.red == 255 && p.green == 0 && p.blue == 0) {
                fputc(PATH, file);
            } else if (p.red == 0 && p.green ==0 && p.blue == 0) {
                fputc(WALL, file);
            } else {
                fputc('?', file);
            }
        }

        fputc('\n', file);
    }

    // Free the image
    for (int r = 0; r < img.height; r++) free(img.rows[r]);
    free(img.rows);
    fclose(file);
}
