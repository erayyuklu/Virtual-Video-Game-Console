#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <fcntl.h>

#define WIDTH 50
#define HEIGHT 20
#define PADDLE_WIDTH 7
#define BRICK_ROWS 4
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




// Custom getch and kbhit implementations
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
    ch =tolower(ch);

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

    return tolower(ch);
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



void exit_game(int signal) {
    (void)signal; // Avoid unused parameter warning
    // Restore terminal settings
    //clear the terminal
    printf("\033[H\033[J");
    disable_raw_mode();
    // Free dynamically allocated memory
    exit(0);
}

void setup_signal_handlers() {
    struct sigaction sa;
    sa.sa_handler = exit_game;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    // Handle SIGINT (Ctrl+C) and SIGTERM signals
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);
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
}

// Draw the game state
void draw_game() {
    printf("\033[H\033[J");

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
    printf("\n");

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
        printf("\033[H\033[JGame Over! Q for exit, R for retry\n");
        k: char choice;
        choice = getch();
        if(choice=='q' || choice == 'r'){
                        
            if (choice == 'r') {
                init_game();
            } else if (choice == 'q') {
                exit_game(0);
            }}
            else{
                goto k;
            }
    }

    // Check for win
    if (bricks_left == 0) {
       
        printf("\033[H\033[JYou Win! Q for exit, R for playing again\n");
        m: char choice;
        choice = getch();
        if(choice=='q' || choice == 'r'){
                        
            if (choice == 'r') {
                init_game();
            } else if (choice == 'q') {
                exit_game(0);
            }}
            else{
                goto m;
            }
    }
}

// Read user input and move paddle
void process_input() {
    if (kbhit()) {
        int c = getch();
        if (c == 'a' && paddle.x > 0) {
            paddle.x -= PADDLE_SPEED;
            if (paddle.x < 0) paddle.x = 0; // Prevent overflow
        }
        if (c == 'd' && paddle.x < WIDTH - PADDLE_WIDTH) {
            paddle.x += PADDLE_SPEED;
            if (paddle.x > WIDTH - PADDLE_WIDTH) paddle.x = WIDTH - PADDLE_WIDTH; // Prevent overflow
        }
        if (c == 'q') {
                exit_game(0);
            }
    }
}

// Main game loop
void game_loop() {
    struct timespec last_update, current_time;
    long delta_time;

    clock_gettime(CLOCK_MONOTONIC, &last_update);

    while (running) {
        draw_game();

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
    enable_raw_mode();
    setup_signal_handlers();
    init_game();
    game_loop();
    return 0;
}