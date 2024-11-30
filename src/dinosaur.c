#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>
#include <sys/time.h>
#include <signal.h>
#include <ctype.h>

#define GAME_WIDTH 60
#define GAME_HEIGHT 8
#define MAX_OBSTACLES 100
#define MAX_JUMP_HEIGHT 4 // Increased jump height
void disable_raw_mode();
int temporary = 1;

struct termios orig_termios;
int selected_button = 0; // 0: Play, 1: Exit
// Enum to manage jump state
typedef enum {
    GROUNDED,
    ASCENDING,
    DESCENDING
} JumpState;

// Struct to manage game state
typedef struct {
    int dino_pos;
    JumpState jump_state;
    int obstacles[MAX_OBSTACLES][3]; // [x_pos, width, height]
    int obstacle_count;
    int score;
    time_t last_obstacle_time;
    int jump_frame_counter;
} GameState;

// Function to get current time in milliseconds
long long get_milliseconds() {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000LL + tv.tv_usec / 1000;
}

// Function to initialize game state
void init_game(GameState *game) {
    game->dino_pos = 0;
    game->jump_state = GROUNDED;
    game->obstacle_count = 0;
    game->score = 0;
    game->last_obstacle_time = 0;
    game->jump_frame_counter = 0;
}



// Function to manage dinosaur jump
void manage_jump(GameState *game) {
    switch (game->jump_state) {
        case GROUNDED:
            // Do nothing
            break;
        
        case ASCENDING:
            game->jump_frame_counter++;
            // Slower ascent
            if (game->jump_frame_counter % 3 == 0) {
                if (game->dino_pos < MAX_JUMP_HEIGHT) {
                    game->dino_pos++;
                    if(game->dino_pos == 1){
                        temporary = 0;
                    }
                    else{
                        temporary = 1;
                    }
                } else {
                    // Reached max height, start descending
                    game->jump_state = DESCENDING;
                }
            }
            break;
        
        case DESCENDING:
            game->jump_frame_counter++;
            // Slower descent
            if (game->jump_frame_counter % 3 == 0) {
                if(game->dino_pos == 1){
                        temporary = 0;
                    }
                    else{
                        temporary = 1;
                    }
                if (game->dino_pos > 0) {
                    game->dino_pos--;
                } else {
                    // Back to ground
                    game->jump_state = GROUNDED;
                    game->jump_frame_counter = 0;
                }
            }
            break;
    }
}

// Function to initiate jump
void start_jump(GameState *game) {
    if (game->jump_state == GROUNDED) {
        game->jump_state = ASCENDING;
        game->jump_frame_counter = 0;
    }
}

// Function to generate obstacles with controlled frequency
void generate_obstacle(GameState *game) {
    time_t current_time = time(NULL);

    // Ensure at least 2 seconds between obstacle generations
    if (difftime(current_time, game->last_obstacle_time) < 1) {
        return;
    }

    // Random chance of generating an obstacle
    if (rand() % 4 == 0 && game->obstacle_count < MAX_OBSTACLES ) { // 25% chance
        game->obstacles[game->obstacle_count][0] = GAME_WIDTH + 2*(rand() % 10); // Random start position slightly beyond screen
        game->obstacle_count++;
        game->last_obstacle_time = current_time;
    }
}

// Function to move obstacles
void move_obstacles(GameState *game) {
    for (int i = 0; i < game->obstacle_count; i++) {
        game->obstacles[i][0]--;

        // Remove obstacles that have moved off screen
        if (game->obstacles[i][0] < 0) {
            for (int j = i; j < game->obstacle_count - 1; j++) {
                game->obstacles[j][0] = game->obstacles[j+1][0];
                game->obstacles[j][1] = game->obstacles[j+1][1];
                game->obstacles[j][2] = game->obstacles[j+1][2];
            }
            game->obstacle_count--;
            i--;
            game->score++;
        }
    }
}

// Function to check collision
int check_collision(GameState *game) {
    for (int i = 0; i < game->obstacle_count; i++) {
        // More precise collision detection for 2x2 dinosaur and 2-height obstacles
        if (game->obstacles[i][0] < 5 && game->obstacles[i][0] > 0 && 
            game->dino_pos < 2) {
            return 1;  // Collision detected
        }
    }
    return 0;
}

// Function to render game state
void render(GameState *game) {
    // Clear the screen
    printf("\033[H\033[J");
    // Add decorative stars at the top
    const char *decorative_lines[] = {
        "                                                ",
        "          ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~    ",
        "   ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~   ",
        "         (^_^)      *     ***    ***    *   *      ",
        "     ~ ~ ~ ~ ~ ~ ~ ~   ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~  ",
        "          ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~    ",
    };

    for (int i = 0; i < 6; i++) { // Print the first 6 decorative lines
        printf("%s\n", decorative_lines[i]);
    }

    // Render game area
    for (int y = 0; y < GAME_HEIGHT; y++) {
        for (int x = 0; x < GAME_WIDTH; x++) {
            int render_dino = 0;
            int render_obstacle_top = 0;
            int render_obstacle_bottom = 0;

            // Check for dinosaur rendering
            if (x == 3 || x == 4) {
                if (y == GAME_HEIGHT - 3 - game->dino_pos) { // Head
                    printf(" O");
                    render_dino = 1;
                } else if (y == GAME_HEIGHT - 2 - game->dino_pos) { // Arms
                    printf(" |");
                    render_dino = 1;
                } else if (y == GAME_HEIGHT - 1 - game->dino_pos) { // Legs
                    printf(" ⋀");
                    render_dino = 1;
                }
            }

            if (temporary==0){

            }

            // Check for obstacle rendering
            for (int i = 0; i < game->obstacle_count; i++) {
                int obstacle_x = game->obstacles[i][0];

                if (x == obstacle_x) {
                    if (y == GAME_HEIGHT - 2) { // Top of obstacle
                        printf("╔╗");
                        render_obstacle_top = 1;
                    } else if (y == GAME_HEIGHT - 1) { // Bottom of obstacle
                        printf("╚╝");
                        render_obstacle_bottom = 1;
                    }
                }
            }

            // Print space if nothing is rendered
            if (!render_dino && !render_obstacle_top && !render_obstacle_bottom) {
                printf(" ");
            }

            // Move to the next position
            if (render_dino || render_obstacle_top || render_obstacle_bottom) {
                x++; // Skip the next column as we render two characters per entity
            }
        }
        printf("\n");
    }
    // Render the ground
    for (int x = 0; x < GAME_WIDTH; x++) {
        printf("▓");
    }
    printf("\n");

    // Display score and jump state
    printf("Score: %d\n", 
           game->score);
}
void handle_signal(int sig) {
    disable_raw_mode();
    printf("\033[H\033[J");
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


int main() {
      // Set up signal handling
    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    // Enable raw mode for terminal input
    enable_raw_mode();
    // Seed random number generator

    srand(time(NULL));

    GameState game;
    int restart_game = 0;

    do {
        init_game(&game);

        // Timing variables for frame rate control
        long long last_frame_time = get_milliseconds();
        const long long FRAME_TIME = 20;  // 20 ms per frame (about 50 FPS)

        while (1) {
            // Frame rate control
            long long current_time = get_milliseconds();
            long long elapsed_time = current_time - last_frame_time;

            // Handle input
            if (kbhit()) {
                char ch = getchar();
                if (ch == ' ') {
                    start_jump(&game);
                }
                if (ch == 'q') {
                    restart_game=0;
                    break;
                }
            }

            // Only update if enough time has passed
            if (elapsed_time >= FRAME_TIME) {
                // Manage jump mechanics
                manage_jump(&game);

                // Generate obstacles
                generate_obstacle(&game);

                // Move obstacles
                move_obstacles(&game);

                // Check for collision
                if (check_collision(&game)) {
                    printf("Game Over! Final Score: %d\n", game.score);
                    printf("Jump for retry or press Q for exit\n");
                    
                    k: char choice;
                    choice = getch();
                    if(choice=='q' || choice == ' '){
                        
                        if (choice == ' ') {
                            restart_game = 1;
                            break;
                        } else if (choice == 'q') {
                            restart_game = 0;
                            break;
                        }}
                    else{
                        goto k;
                        break;
                    }
                    
                }

                // Render game state
                render(&game);

                // Update last frame time
                last_frame_time = current_time;
            }
            else {
                // Small sleep to prevent CPU overuse
                usleep(10000);  // 10ms sleep
            }
        }
    } while (restart_game);
    // Restore terminal settings before exiting
    printf("\033[H\033[J");
    disable_raw_mode();

    return 0;
}