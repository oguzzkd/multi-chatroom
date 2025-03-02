enum colors {
    RED,
    GREEN,
    YELLOW,
    BLUE,
    PURPLE,
    CYAN,
    RESET_COLOR, // colors after RESET_COLOR won't be used for usernames
    LIGHT_BLUE
};

#define COLOR_COUNT (RESET_COLOR - RED)

extern char * color_codes[];

char * formatColor(enum colors color, char src[], char dest[], size_t dest_size);