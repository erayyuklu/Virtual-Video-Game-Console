#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <signal.h>
#include <ctype.h>

#define ROWS 15
#define COLS 15

typedef struct {
    int x, y;
} Point;

Point *snake = NULL;
int snake_length = 2;
int snake_capacity;
Point food;
char direction = 'a';

// Terminal configuration
struct termios orig_termios;

// Function prototypes
void initialize_game();
void draw_board();
void generate_food();
void update_snake(char input);
bool is_collision(Point next_head);
void wait_for_valid_input();
int kbhit();
char getch();
void delay(int milliseconds);
void enable_raw_mode();
void disable_raw_mode();
void exit_game(int signal);
void setup_signal_handlers();

int main() {
    enable_raw_mode();
    setup_signal_handlers();
    initialize_game();

    while (1) {
        draw_board();

        if (kbhit()) {
            char input = getch();
            if (input == 'q') {
                exit_game(0);
            }
            input = tolower(input);
            if ((input == 'w' && direction != 's') ||
                (input == 's' && direction != 'w') ||
                (input == 'a' && direction != 'd') ||
                (input == 'd' && direction != 'a')) {
                direction = input;
            }
        }

        update_snake(direction);
        delay(150);
    }

    disable_raw_mode();
    return 0;
}

void initialize_game() { // Initialize the game state
    snake_capacity = ROWS * COLS;
    snake = (Point *)malloc(snake_capacity * sizeof(Point));
    if (!snake) {
        perror("Failed to allocate memory for snake");
        exit(EXIT_FAILURE);
    }

    snake_length = 2;

    for (int i = 0; i < snake_capacity; i++) {
        snake[i].x = -1;
        snake[i].y = -1;
    }

    snake[0].x = ROWS / 2;
    snake[0].y = COLS / 2;
    snake[1].x = ROWS / 2;
    snake[1].y = (COLS / 2) - 1;

    generate_food();
}

void draw_board() { // Draw the game state
    printf("\033[H\033[J");

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            int is_snake = 0, is_head = 0;

            if (snake[snake_length - 1].x == i && snake[snake_length - 1].y == j) {
                is_head = 1;
                printf("O ");
            }

            if (!is_head) {
                for (int k = 0; k < snake_length - 1; k++) {
                    if (snake[k].x == i && snake[k].y == j) {
                        is_snake = 1;
                        printf("# ");
                        break;
                    }
                }
            }

            if (!is_snake && !is_head) {
                if (food.x == i && food.y == j) {
                    printf("X ");
                } else {
                    printf(". ");
                }
            }
        }
        printf("\n");
    }
}

void generate_food() { // Generate food at a random location
    srand(time(NULL));
    while (1) {
        food.x = rand() % ROWS;
        food.y = rand() % COLS;

        int on_snake = 0;
        for (int i = 0; i < snake_length; i++) {
            if (snake[i].x == food.x && snake[i].y == food.y) {
                on_snake = 1;
                break;
            }
        }

        if (!on_snake) break;
    }
}

bool is_collision(Point next_head) { // Check for collision with walls or itself
    if (next_head.x < 0 || next_head.x >= ROWS || next_head.y < 0 || next_head.y >= COLS) {
        return true;
    }

    for (int i = 0; i < snake_length; i++) {
        if (snake[i].x == next_head.x && snake[i].y == next_head.y) {
            return true;
        }
    }

    return false;
}

void wait_for_valid_input() { // Wait for valid input to change direction
    while (1) {
        if (kbhit()) {
            char new_input = getch();
            new_input = tolower(new_input);

            Point new_next_head = snake[snake_length - 1]; // Current head position

            // Determine potential new head position based on input
            if (new_input == 'w') new_next_head.x--;
            else if (new_input == 'a') new_next_head.y--;
            else if (new_input == 's') new_next_head.x++;
            else if (new_input == 'd') new_next_head.y++;
            else if (new_input == 'q') exit_game(0);

            // Check if the cell is valid (inside the board and empty)
            bool is_valid_cell = 
                new_next_head.x >= 0 && new_next_head.x < ROWS &&
                new_next_head.y >= 0 && new_next_head.y < COLS;

            if (is_valid_cell) {
                // Check if the cell is empty (".")
                bool is_empty_cell = true;
                for (int i = 0; i < snake_length; i++) {
                    if (snake[i].x == new_next_head.x && snake[i].y == new_next_head.y) {
                        is_empty_cell = false;
                        break;
                    }
                }

                if (is_empty_cell) {
                    // Set direction and exit
                    direction = new_input;
                    break;
                }
            }
        }
    }
}


void update_snake(char input) { // Update the snake's position
    Point next_head = snake[snake_length - 1];

    if (input == 'w') next_head.x--;
    else if (input == 'a') next_head.y--;
    else if (input == 's') next_head.x++;
    else if (input == 'd') next_head.y++;

    if (is_collision(next_head)) {
        wait_for_valid_input();
        return;
    }

    bool ate_food = (next_head.x == food.x && next_head.y == food.y);

    if (ate_food) { // Increase snake length and generate new food
        if (snake_length >= snake_capacity) {
            snake_capacity *= 2;
            snake = (Point *)realloc(snake, snake_capacity * sizeof(Point));
            if (!snake) {
                perror("Failed to reallocate memory for snake");
                exit(EXIT_FAILURE);
            }
        }
        snake_length++;
        generate_food();
    } else {
        for (int i = 0; i < snake_length - 1; i++) {
            snake[i] = snake[i + 1];
        }
    }

    snake[snake_length - 1] = next_head;
}

int kbhit() { // Check if a key has been pressed
    struct termios oldt, newt;
    int ch, oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

char getch() { // Read a character from the input
    struct termios oldt, newt;
    char ch;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return tolower(ch);
}

void delay(int milliseconds) {// Delay the program
    usleep(milliseconds * 1000);
}

void enable_raw_mode() { // Enable raw mode for terminal input
    struct termios raw;

    tcgetattr(STDIN_FILENO, &orig_termios);
    raw = orig_termios;

    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disable_raw_mode() { // Disable raw mode and restore terminal settings
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

void exit_game(int signal) { // Exit the game
    (void)signal;
    disable_raw_mode();
    free(snake);
    exit(0);
}

void setup_signal_handlers() { // Set up signal handlers for SIGINT and SIGTERM
    struct sigaction sa;
    sa.sa_handler = exit_game;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}
