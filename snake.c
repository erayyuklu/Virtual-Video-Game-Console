#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdbool.h>
#include <signal.h>

#define ROWS 15
#define COLS 15

// Snake structure
typedef struct {
    int x, y;
} Point;

Point snake[ROWS * COLS];
int snake_length = 2;
Point food;
char direction = 'd';

// Terminal configuration
struct termios orig_termios;

void initialize_game();
void draw_board();
void generate_food();
void update_snake(char input);
int check_collision();
int kbhit();
char getch();
void delay(int milliseconds);
void enable_raw_mode();
void disable_raw_mode();
void exit_game(int signal);

// Signal handler setup for graceful exit
void setup_signal_handlers();

int main() {
    // Enable raw mode for terminal input
    enable_raw_mode();
    
    // Setup signal handlers for SIGINT and SIGTERM
    setup_signal_handlers();

    initialize_game();

    while (1) {
        draw_board();
        if (kbhit()) {
            char input = getch();
            if (input == 'w' || input == 'a' || input == 's' || input == 'd') {
                direction = input;
            }
            if (input == 'q') {
                // Gracefully exit the game when 'q' is pressed
                exit_game(0);
            }
        }
        update_snake(direction);

        if (check_collision()) {
            direction = 'p'; // Stop the snake until a valid input
        }

        delay(300);
    }

    // Restore terminal settings before exiting
    disable_raw_mode();
    return 0;
}

void initialize_game() {
    // Initialize snake length and position
    snake_length = 2;

    // Set all positions in the snake array to an invalid state (-1, -1)
    for (int i = 0; i < ROWS * COLS; i++) {
        snake[i].x = -1;
        snake[i].y = -1;
    }

    // Initialize the first two segments of the snake
    snake[0].x = ROWS / 2;
    snake[0].y = COLS / 2;
    snake[1].x = ROWS / 2;
    snake[1].y = (COLS / 2) - 1;

    // Generate initial food
    generate_food();
}

void draw_board() {
    system("clear"); // Clear the terminal screen

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            int is_snake = 0;
            int is_head = 0;

            // Check if the current cell is the snake's head
            if (snake[snake_length - 1].x == i && snake[snake_length - 1].y == j) {
                is_head = 1;
                printf("O "); // Snake's head
            }

            // Check if the current cell is part of the snake's body
            if (!is_head) { // Only check for body if it's not the head
                for (int k = 0; k < snake_length - 1; k++) {
                    if (snake[k].x == i && snake[k].y == j) {
                        is_snake = 1;
                        printf("# "); // Snake's body
                        break;
                    }
                }
            }

            // Print the food or empty cell if not part of the snake
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

void generate_food() {
    srand(time(NULL));
    while (1) {
        food.x = rand() % ROWS;
        food.y = rand() % COLS;

        // Ensure food doesn't spawn on the snake
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

void update_snake(char input) {
    Point next_head = snake[snake_length - 1]; // Determine next head position

    // Update the next head position based on input
    if (input == 'w') next_head.x--;
    else if (input == 'a') next_head.y--;
    else if (input == 's') next_head.x++;
    else if (input == 'd') next_head.y++;

    // Wrap the head position around the board edges
    next_head.x = (next_head.x + ROWS) % ROWS; // Ensure x is within 0 to ROWS-1
    next_head.y = (next_head.y + COLS) % COLS; // Ensure y is within 0 to COLS-1

    // Check if food is eaten
    bool ate_food = (next_head.x == food.x && next_head.y == food.y);

    if (ate_food) {
        // Grow the snake by not shifting the tail
        snake_length++;
        generate_food(); // Generate new food
    } else {
        // Shift the snake's body only if no food was eaten
        for (int i = 0; i < snake_length - 1; i++) {
            snake[i] = snake[i + 1];
        }
    }

    // Add the new head position
    snake[snake_length - 1] = next_head;
}



int check_collision() {
    Point head = snake[snake_length - 1];

    // Allow the snake to pass over itself; no need to stop or treat this as a collision.
    return 0; // Always return 0 to ignore self-collision.
}

int kbhit() {
    struct termios oldt, newt;
    int ch;
    int oldf;

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

char getch() {
    struct termios oldt, newt;
    char ch;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    ch = getchar();
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);

    return ch;
}

void delay(int milliseconds) {
    usleep(milliseconds * 500);
}

void enable_raw_mode() {
    struct termios raw;

    // Get current terminal settings
    tcgetattr(STDIN_FILENO, &orig_termios);
    raw = orig_termios;

    // Disable canonical mode and echo
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

void disable_raw_mode() {
    // Restore original terminal settings
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}

// Gracefully exit the game
void exit_game(int signal) {
    (void) signal; // Avoid unused parameter warning
    printf("\nExiting the game...\n");

    // Restore terminal settings
    disable_raw_mode();

    exit(0);
}

// Set up signal handlers for SIGINT and SIGTERM
void setup_signal_handlers() {
    struct sigaction sa;
    sa.sa_handler = exit_game;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    // Handle SIGINT (Ctrl+C) and SIGTERM signals
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
}
