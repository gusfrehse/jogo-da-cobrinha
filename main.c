#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include <SDL2/SDL.h>
#include <GL/gl.h>

// Define MAX and MIN macros
#define MAX(X, Y) (((X) > (Y)) ? (X) : (Y))
#define MIN(X, Y) (((X) < (Y)) ? (X) : (Y))

// Define screen dimensions
#define SCREEN_WIDTH    600
#define SCREEN_HEIGHT   600

#define BOARD_WIDTH 100
#define BOARD_HEIGHT 100


/**
HELPER FUNCTIONS
**/

// retorna tempo atual em segundos
float current_time() {
    struct timespec time;
    clock_gettime(CLOCK_MONOTONIC, &time);

    return time.tv_sec + (time.tv_nsec / 1e9);
}

/**

CORES

grama: 53, 112, 37
cabeca: 48, 199, 6
corpo: 37, 163, 2
azeitona: 21, 59, 11
parede: 4, 18, 0

*/

const float block_width = 1.0 / BOARD_WIDTH;
const float block_height = 1.0 / BOARD_HEIGHT;

enum board_entry {
    NOTHING,
    WALL,
    FOOD,
    SNAKE
};

int board[BOARD_HEIGHT][BOARD_WIDTH];

struct body {
    int x, y;
    struct body *next;
    struct body *previous;
};

struct food {
    int x, y;
} food;

enum direction { UP, RIGHT, DOWN, LEFT, NONE };

struct snake {
    struct body *head; 
    struct body *tail; 
    int size;
    enum direction direction;
} snake = { NULL };

enum direction next_direction;

struct body body_arena[BOARD_HEIGHT * BOARD_WIDTH];
int next_body = 0;

struct body *alloc_body() {
    return body_arena + next_body++;
}

void init() {
    // create head
    snake.size = 2;
    snake.head = alloc_body();
    snake.head->x = BOARD_WIDTH / 2;
    snake.head->y = BOARD_HEIGHT / 2;
    snake.head->next = NULL;
    snake.head->previous = NULL;

    // the head is the tail at first
    snake.tail = alloc_body();
    snake.tail->x = snake.head->x - 1;
    snake.tail->y = snake.head->y;
    snake.tail->next = NULL;
    snake.tail->previous = snake.head;

    snake.head->next = snake.tail;

    snake.direction = RIGHT;
    
    board[snake.head->x][snake.head->y] = SNAKE;

    food.x = rand() % BOARD_WIDTH;
    food.y = rand() % BOARD_HEIGHT;
}


void render_snake() {
    struct body *current = snake.head;

    while (current) {
        glColor3f(48.0 / 255.0, 199.0 / 255.0, 6.0 / 255.0);

        float x = block_width * current->x;
        float y = block_height * current->y;
        fprintf(stderr, "rendering body x %g y %g width %g height %g\n", x, y, block_width, block_height);

        // render head
        glBegin(GL_QUADS); // Use GL_LINE_LOOP for just the outline
        glVertex2f(x, y);
        glVertex2f(x + block_width, y);
        glVertex2f(x + block_width, y + block_height);
        glVertex2f(x, y + block_height);
        glEnd();

        current = current->next;
    }
}

void render_food() {
    glColor3f(21.0 / 255.0, 59.0 / 255.0, 11.0 / 255.0);

    float x = block_width * food.x;
    float y = block_height * food.y;
    fprintf(stderr, "rendering food at x %g y %g width %g height %g\n", x, y, block_width, block_height);

    // render head
    glBegin(GL_QUADS); // Use GL_LINE_LOOP for just the outline
    glVertex2f(x, y);
    glVertex2f(x + block_width, y);
    glVertex2f(x + block_width, y + block_height);
    glVertex2f(x, y + block_height);
    glEnd();
}

void render_wall() {
}

void render() {
    fprintf(stderr, "RENDER\n");
    glClearColor(53.0 / 255.0, 112.0 / 255.0, 37.0 / 255.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, 1, 0, 1, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
   
    render_snake();
    render_food();
}


struct coord { int x, y; };
void step() {
    struct coord direction_to_coords[] = { { 0, 1 }, { 1, 0 }, { 0, -1 }, { -1, 0 } };

    if (next_direction != NONE)
    {
        snake.direction = next_direction;
        next_direction = NONE;
    }

    struct coord diff = direction_to_coords[snake.direction];
    
    int next_x = snake.head->x + diff.x;
    int next_y = snake.head->y + diff.y;
    
    if (snake.size == 1) {
        snake.head->x = next_x;
        snake.head->y = next_y;
    } else {
        struct body *new_head = snake.tail;
        struct body *new_tail = snake.tail->previous;
        
        new_head->next = snake.head;
        new_head->previous = NULL;
        snake.head->previous = new_head;
        snake.head = new_head;

        if (new_tail)
            new_tail->next = NULL;
        snake.tail = new_tail;
        
        snake.head->x = next_x;
        snake.head->y = next_y;
    }
}

int main(int, char*[])
{
    srand(time(NULL));    

    // Initialize SDL
    if(SDL_Init(SDL_INIT_VIDEO) < 0)
    {
        fprintf(stderr, "SDL could not be initialized!\n"
               "SDL_Error: %s\n", SDL_GetError());
        return 0;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // Create window
    SDL_Window *window = SDL_CreateWindow("jogo",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SCREEN_WIDTH, SCREEN_HEIGHT,
                                          SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    if(!window)
    {
        fprintf(stderr, "Window could not be created!\n"
               "SDL_Error: %s\n", SDL_GetError());
    }

    SDL_GLContext context = SDL_GL_CreateContext(window);
    
    if (!window)
    {
        fprintf(stderr, "Failed to create context: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    SDL_GL_SetSwapInterval(1);

    init();

    const float step_time = 1.0 / 8.0;
    float time_since_last_step = 0.0f;
    float previous_time = current_time();

    int running = 1;
    SDL_Event e;

    while (running)
    {
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_QUIT)
            {
                running = 0;
            }
            else if (e.type == SDL_KEYDOWN)
            {
              if (e.key.keysym.sym == SDLK_UP)
              {
                next_direction = UP;
              } else if (e.key.keysym.sym == SDLK_RIGHT)
              {
                next_direction = RIGHT;
              } else if (e.key.keysym.sym == SDLK_DOWN)
              {
                next_direction = DOWN;
              } else if (e.key.keysym.sym == SDLK_LEFT)
              {
                next_direction = LEFT;
              }
            }
            else if (e.type == SDL_KEYUP)
            {
            }
        }
        
        if (time_since_last_step > step_time) {
            fprintf(stderr, "STEP\n");
            step();
            time_since_last_step = 0.0f;
        }   

        render();

        SDL_GL_SwapWindow(window);
        
        float time = current_time();
        time_since_last_step += time - previous_time;
        previous_time = time;
    }

    // Destroy window
    SDL_DestroyWindow(window);

    // Quit SDL
    SDL_Quit();

    return 0;
}
