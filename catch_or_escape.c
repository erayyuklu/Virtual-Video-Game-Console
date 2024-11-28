#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#define ROWS 15
#define COLS 30

char screen[ROWS][COLS];
int stick_pos = COLS / 2 - 1; // Initial stick position
int game_over = 0;
int item_timer = 0;

// Terminal configuration
struct termios orig_termios;

// Function prototypes
void initialize_game();
void draw_screen();
void generate_item();
void update_items();
void check_collision();
int kbhit();
char getch();
void delay(int milliseconds);
void enable_raw_mode();
void disable_raw_mode();
void handle_input();
void exit_game(int signal);

// Main function
int main() {
    enable_raw_mode();

    // Set up signal handlers
    struct sigaction sa;
    sa.sa_handler = exit_game;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
    sigaction(SIGTERM, &sa, NULL);

    srand(time(NULL)); // Seed for random item generation
    initialize_game();

    while (!game_over) {
        if (kbhit()) {
            handle_input();
        }
        if (item_timer >= (2 + rand() % 3) * 500) { // Generate items every 2-4 seconds
            generate_item();
            item_timer = 0;
        }
        update_items();
        check_collision();
        draw_screen();
        delay(200);
        item_timer += 200;
    }

    printf("Game Over!\n");
    disable_raw_mode();
    return 0;
}

// Initialize the game screen
void initialize_game() {
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            screen[i][j] = ' ';
        }
    }

    // Place the stick on the bottom row
    for (int i = 0; i < 3; i++) {
        screen[ROWS - 1][stick_pos + i] = '=';
    }
}

// Draw the game screen
void draw_screen() {
    system("clear");
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            printf("%c", screen[i][j]);
        }
        printf("\n");
    }
}

// Generate a random item at the top
void generate_item() {
    int col = rand() % COLS;
    if (screen[0][col] == ' ') {
        screen[0][col] = (rand() % 2 == 0) ? 'O' : 'X'; // Randomly choose 'O' or 'X'
    }
}

// Update the position of items
void update_items() {
    for (int i = ROWS - 2; i >= 0; i--) { // Start from the second-to-last row
        for (int j = 0; j < COLS; j++) {
            if (screen[i][j] == 'O' || screen[i][j] == 'X') {
                screen[i + 1][j] = screen[i][j];
                screen[i][j] = ' ';
            }
        }
    }
}

void check_collision() {
    int missed_good_item = 1;  // Flag to track if we missed a good item (O)

    // Iterate through the columns to check collisions with the stick
    for (int j = 0; j < COLS; j++) {
        // Check if the stick touches an 'X'
        if ((screen[ROWS - 1][j] == 'X') && (j >= stick_pos - 1 && j <= stick_pos + 1)) {
            game_over = 1; // Game over if the stick touches 'X'
        }

        // Check if the stick catches an 'O'
        if ((screen[ROWS - 1][j] == 'O') && (j >= stick_pos - 1 && j <= stick_pos + 1)) {
            screen[ROWS - 1][j] = ' '; // Remove the 'O' after catching it
            missed_good_item = 0; // We caught a good item
        }
    }

    // After checking for collisions, move all items down one row
    for (int i = ROWS - 2; i >= 0; i--) {
        for (int j = 0; j < COLS; j++) {
            screen[i + 1][j] = screen[i][j];  // Move item down
        }
    }

    // Clear the top row
    for (int j = 0; j < COLS; j++) {
        screen[0][j] = ' ';
    }

    // If an 'O' is missed (it falls past the stick without being caught)
    if (missed_good_item) {
        for (int i = stick_pos - 1; i <= stick_pos + 1; i++) {
            if (screen[ROWS - 1][i] == 'O') {
                game_over = 1; // Missed a good item (O) if it passed below the stick without being caught
                break;
            }
        }
    }

    // Now delete any items that have reached the bottom row (ground) and were missed
    for (int j = 0; j < COLS; j++) {
        if (screen[ROWS - 1][j] == 'O' || screen[ROWS - 1][j] == 'X') {
            screen[ROWS - 1][j] = ' ';  // Clear the item that reached the ground
        }
    }
}



// Handle user input
void handle_input() {
    char input = getch();
    if (input == 'a' && stick_pos > 1) {
        // Move stick left faster (2 columns)
        screen[ROWS - 1][stick_pos + 2] = ' ';
        screen[ROWS - 1][stick_pos + 1] = ' ';
        stick_pos -= 2;
        screen[ROWS - 1][stick_pos] = '=';
        screen[ROWS - 1][stick_pos + 1] = '=';
        screen[ROWS - 1][stick_pos + 2] = '=';
    } else if (input == 'd' && stick_pos < COLS - 5) {
        // Move stick right faster (2 columns)
        screen[ROWS - 1][stick_pos] = ' ';
        screen[ROWS - 1][stick_pos + 1] = ' ';
        stick_pos += 2;
        screen[ROWS - 1][stick_pos] = '=';
        screen[ROWS - 1][stick_pos + 1] = '=';
        screen[ROWS - 1][stick_pos + 2] = '=';
    } else if (input == 'q') {
        game_over = 1; // Quit the game
    }
}

// Exit the game gracefully
void exit_game(int signal) {
    (void) signal; // Suppress unused warning
    disable_raw_mode();
    printf("\nExiting the game...\n");
    exit(0);
}

// Check for keyboard input
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

// Get a single character from input
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

// Delay in milliseconds
void delay(int milliseconds) {
    usleep(milliseconds * 1000);
}

// Enable raw terminal mode
void enable_raw_mode() {
    struct termios raw;

    tcgetattr(STDIN_FILENO, &orig_termios);
    raw = orig_termios;
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &raw);
}

// Disable raw terminal mode
void disable_raw_mode() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &orig_termios);
}
