#include <string.h>

#include "colors.h"

char * color_codes[] = {
    // ANSI Standard
    "\e[0;31m",
    "\e[0;32m",
    "\e[0;33m",
    "\e[0;34m",
    "\e[0;35m",
    "\e[0;36m",
    "\e[0m",
    "\e[38;5;195m"
};

char * formatColor(enum colors color, char src[], char dest[], size_t dest_size){
    strncat(dest, color_codes[color], dest_size - strlen(dest));
    dest[dest_size - 1] = '\0';

    strncat(dest, src, dest_size - strlen(dest));
    dest[dest_size - 1] = '\0';
    
    strncat(dest, color_codes[RESET_COLOR], dest_size - strlen(dest));
    dest[dest_size - 1] = '\0';

    return dest;
}