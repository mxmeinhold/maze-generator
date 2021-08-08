/**
 * format.h - defines some macros for ANSI formatting escape sequences
 */

#ifndef MAZE_GEN_FORMAT_H
#define MAZE_GEN_FORMAT_H

#define BOLD "\033[1m"
#define FAINT "\033[2m"
#define UNDERLINE "\033[4m"
/** Neither bold nor faint */
#define INTENSITY_RESET "\033[22m"
#define UNDERLINE_OFF "\033[24m"
#define RESET "\033[0m"
#define TAB "    "

#endif
