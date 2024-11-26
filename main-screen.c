#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_GAMES 100
#define MAX_NAME_LEN 256

// Terminal configuration
struct termios orig_termios;

// Game list and selected index
char games[MAX_GAMES][MAX_NAME_LEN];
int num_games = 0;
int selected_game = 0;

void enable_raw_mode();
void disable_raw_mode();
void handle_exit(int signum);
void load_games();
void draw_screen();
int kbhit();
char getch();
void delay(int milliseconds);

int main() {
    // Setup signal handlers for graceful exit
    signal(SIGINT, handle_exit);
    signal(SIGTERM, handle_exit);

    // Enable raw mode for terminal input
    enable_raw_mode();

    // Load games
    load_games();

    // Main loop
    while (1) {
        draw_screen();

        if (kbhit()) {
            char input = getch();

            if (input == 'q') {
                break; // Quit the main menu
            } else if (input == 'd') {
                selected_game = (selected_game + 1) % num_games; // Next game
            } else if (input == 'a') {
                selected_game = (selected_game - 1 + num_games) % num_games; // Previous game
            } else if (input == '\n') { // Enter key
                char command[MAX_NAME_LEN + 10];
                snprintf(command, sizeof(command), "./%s", games[selected_game]);
                disable_raw_mode();
                system(command); // Execute the selected game
                enable_raw_mode();
            }
        }

        delay(200); // Slow down the loop
    }

    // Restore terminal settings before exiting
    disable_raw_mode();
    return 0;
}

void load_games() {
    struct dirent *entry;
    DIR *dp = opendir(".");

    if (dp == NULL) {
        perror("opendir");
        exit(1);
    }

    while ((entry = readdir(dp))) {
        if (strncmp(entry->d_name, "game_", 5) == 0) {
            strncpy(games[num_games], entry->d_name, MAX_NAME_LEN - 1);
            games[num_games][MAX_NAME_LEN - 1] = '\0';
            num_games++;
            if (num_games >= MAX_GAMES) break;
        }
    }

    closedir(dp);

    if (num_games == 0) {
        printf("No games found starting with 'game_'. Exiting...\n");
        exit(1);
    }
}

void draw_screen() {
    system("clear"); // Clear the terminal screen

    printf("====== MAIN MENU ======\n");
    printf("Use 'a' and 'd' to navigate between games.\n");
    printf("Press 'Enter' to play the selected game.\n");
    printf("Press 'q' to quit.\n");
    printf("=======================\n\n");

    printf("Selected Game: [%s]\n", games[selected_game]);
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

void handle_exit(int signum) {
    disable_raw_mode();
    printf("\nExiting gracefully...\n");
    exit(0);
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
    usleep(milliseconds * 1000);
}
