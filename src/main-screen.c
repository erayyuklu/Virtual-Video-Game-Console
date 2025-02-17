#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <ctype.h>

#define MAX_GAMES 100
#define MAX_NAME_LEN 256



struct termios orig_termios;
char games[MAX_GAMES][MAX_NAME_LEN];
int game_count = 0;
int selected_game = 0;
int selected_button = 0; // 0: Play, 1: Exit

void enable_raw_mode();
void disable_raw_mode();
void handle_signal(int sig);
void scan_games();
void draw_menu();
void execute_game(char[MAX_NAME_LEN]);
int kbhit();
char getch();
void delay(int milliseconds);
char* remove_game_prefix(char* input);

int main() {
    // Set up signal handling
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    // Enable raw mode for terminal input
    enable_raw_mode();

    // Scan for games in the directory
    scan_games();

    while (1) {
        draw_menu();

        if (kbhit()) {
            char input = getch();

            if (input == 'w' && selected_game > 0) {
                selected_game--;
            } else if (input == 'w' && selected_game == 0) {
                selected_game=game_count-1;
            } else if (input == 's' && selected_game < game_count - 1) {
                selected_game++;
            }
            else if (input == 's' && selected_game == game_count - 1) {
                selected_game=0;
            }  else if (input == 'a') {
                selected_button = 0; // Select "Play" button
            } else if (input == 'd') {
                selected_button = 1; // Select "Exit" button
            } else if (input == '\n') { // Enter key
                if (selected_button == 0) {
                    execute_game(games[selected_game]);
                } else if (selected_button == 1) {
                    break;
                }
            } else if (input == 'q') {
                break;
            }
        }

        delay(100);
    }

    // Restore terminal settings before exiting
    disable_raw_mode();
    system("clear");
    return 0;
}

void scan_games() { // Scan for games in the current directory
    DIR *d;
    struct dirent *dir;
    d = opendir("./");
    if (d) {
        while ((dir = readdir(d)) != NULL) {
            if (strncmp(dir->d_name, "game_", 5) == 0) {
                strncpy(games[game_count], dir->d_name, MAX_NAME_LEN - 1);
                games[game_count][MAX_NAME_LEN - 1] = '\0';
                game_count++;
                if (game_count >= MAX_GAMES) break;
            }
        }
        closedir(d);
    }
}

void draw_menu() { // Draw the main menu
    system("clear");
    printf("============ Virtual Console Main Menu ============\n");
    printf("\n");

    for (int i = 0; i < game_count; i++) {
        if (i == selected_game) {
            printf("                 -> %s\n", remove_game_prefix(games[i])); // Highlight selected game
        } 
    }

    printf("\n               [Play] %s  [Exit]\n", selected_button == 0 ? "<-" : "->");
    printf("\nControls: W/S to navigate games, A/D to switch buttons, Enter to select, Q to quit\n");
}

char* remove_game_prefix(char input[MAX_NAME_LEN]) {
    // Find the first occurrence of the underscore character
    char *underscore_position = strchr(input, '_');
    
    // Return the string starting right after the underscore
    return underscore_position + 1;
}


void execute_game(char game_name[MAX_NAME_LEN]) {
    system("clear");
    printf("Launching %s...\n", game_name);
    char command[MAX_NAME_LEN + 10];
    snprintf(command, sizeof(command), "./%s", games[selected_game]);
    disable_raw_mode();
    system(command); // Execute the selected game
    enable_raw_mode();
}

void handle_signal(int sig) {
    disable_raw_mode();
    system("clear");
    exit(0);
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

int kbhit() { // Check if a key has been pressed
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
    ch=tolower(ch);

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

void delay(int milliseconds) { // Delay the program
    usleep(milliseconds * 1000);
}
