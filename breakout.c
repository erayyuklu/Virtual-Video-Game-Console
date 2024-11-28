#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <string.h>
#include <time.h>

#define WIDTH 50
#define HEIGHT 20
#define PADDLE_WIDTH 7
#define BRICK_ROWS 5
#define BRICK_COLS 3
#define BRICK_WIDTH (WIDTH / BRICK_COLS)
#define PADDLE_SPEED 2  // Number of spaces paddle moves per key press

typedef struct {
    int x, y, dx, dy;
} Ball;

typedef struct {
    int x;
} Paddle;

typedef struct {
    int bricks[BRICK_ROWS][BRICK_COLS];
} BrickWall;

Ball ball;
Paddle paddle;
BrickWall wall;
int running = 1;
int bricks_left; // Count of remaining bricks

// Terminal control functions
struct termios orig_termios;

void reset_terminal_mode() {
    tcsetattr(STDIN_FILENO, TCSANOW, &orig_termios);
}

void set_raw_mode() {
    struct termios raw;
    tcgetattr(STDIN_FILENO, &orig_termios);
    atexit(reset_terminal_mode);
    raw = orig_termios;
    raw.c_lflag &= ~(ECHO | ICANON);
    raw.c_cc[VMIN] = 0;
    raw.c_cc[VTIME] = 0; // Non-blocking input
    tcsetattr(STDIN_FILENO, TCSANOW, &raw);
}

// Signal handling
void handle_exit(int sig) {
    running = 0;
}

// Initialize the game state
void init_game() {
    ball.x = WIDTH / 2;
    ball.y = HEIGHT / 2;
    ball.dx = 1;
    ball.dy = -1;

    paddle.x = WIDTH / 2 - PADDLE_WIDTH / 2;

    memset(wall.bricks, 1, sizeof(wall.bricks)); // Fill all bricks with "1" (exists)
    bricks_left = BRICK_ROWS * BRICK_COLS;

    signal(SIGINT, handle_exit);
    signal(SIGTERM, handle_exit);
    set_raw_mode();
}

// Draw the game state
void draw_game() {
    system("clear");

    // Draw bricks
    for (int i = 0; i < BRICK_ROWS; i++) {
        for (int j = 0; j < BRICK_COLS; j++) {
            if (wall.bricks[i][j]) {
                for (int k = 0; k < BRICK_WIDTH - 1; k++) {
                    printf("\033[%d;%dH#", i + 1, j * BRICK_WIDTH + k + 1);
                }
            }
        }
    }

    // Draw ball
    printf("\033[%d;%dHO", ball.y + 1, ball.x + 1);

    // Draw paddle
    for (int i = 0; i < PADDLE_WIDTH; i++) {
        printf("\033[%d;%dH=", HEIGHT, paddle.x + i + 1);
    }

    fflush(stdout);
}

// Update game state
void update_game() {
    // Move ball
    ball.x += ball.dx;
    ball.y += ball.dy;

    // Ball collision with walls
    if (ball.x <= 0 || ball.x >= WIDTH - 1) ball.dx *= -1;

    // Ball collision with top edge
    if (ball.y <= 0) ball.dy *= -1;

    // Ball collision with paddle
    if (ball.y == HEIGHT - 1 && ball.x >= paddle.x && ball.x < paddle.x + PADDLE_WIDTH) {
        ball.dy *= -1;
    }

    // Ball collision with bricks
    if (ball.y < BRICK_ROWS) {
        int row = ball.y;
        int col = ball.x / BRICK_WIDTH;
        if (col >= 0 && col < BRICK_COLS && wall.bricks[row][col]) {
            wall.bricks[row][col] = 0;
            bricks_left--;
            ball.dy *= -1;
        }
    }

    // Check for game over
    if (ball.y >= HEIGHT) {
        running = 0;
        printf("\033[H\033[JGame Over!\n");
    }

    // Check for win
    if (bricks_left == 0) {
        running = 0;
        printf("\033[H\033[JYou Win!\n");
    }
}

// Read user input and move paddle
void process_input() {
    char c;
    while (read(STDIN_FILENO, &c, 1) > 0) {
        if (c == 'a' && paddle.x > 0) {
            paddle.x -= PADDLE_SPEED;
            if (paddle.x < 0) paddle.x = 0; // Prevent overflow
        }
        if (c == 'd' && paddle.x < WIDTH - PADDLE_WIDTH) {
            paddle.x += PADDLE_SPEED;
            if (paddle.x > WIDTH - PADDLE_WIDTH) paddle.x = WIDTH - PADDLE_WIDTH; // Prevent overflow
        }
        if (c == 'q') running = 0;
    }
}

// Main game loop
void game_loop() {
    struct timespec last_update, current_time;
    long delta_time;

    clock_gettime(CLOCK_MONOTONIC, &last_update);

    while (running) {
        draw_game();

        // Poll for input more frequently
        process_input();

        // Update game state only on regular intervals
        clock_gettime(CLOCK_MONOTONIC, &current_time);
        delta_time = (current_time.tv_sec - last_update.tv_sec) * 1000000000L + (current_time.tv_nsec - last_update.tv_nsec);

        if (delta_time >= 150000000L) { // Update every 50ms
            update_game();
            last_update = current_time;
        }

        usleep(10000); // Sleep to reduce CPU usage
    }
}

int main() {
    init_game();
    game_loop();
    return 0;
}
