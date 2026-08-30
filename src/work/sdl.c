#define SDL_MAIN_HANDLED

#include <SDL.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    // 1. Initialize SDL2 Subsystems
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
        return 1;
    }

    // 2. Create the Window Interface
    SDL_Window *window = SDL_CreateWindow(
        "SDL2 Interface Example",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        800, 600,
        SDL_WINDOW_SHOWN);

    // 3. Create the Renderer Interface (Handles drawing)
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // 4. Main Event Loop
    bool isRunning = true;
    SDL_Event event;

    while (isRunning) {
        // Handle input events from the OS interface
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                isRunning = false;
            }
        }

        // Clear interface screen (Set background color to Dark Gray)
        SDL_SetRenderDrawColor(renderer, 40, 40, 40, 255);
        SDL_RenderClear(renderer);

        // Present the double-buffered frame to the display
        SDL_RenderPresent(renderer);
    }

    // 5. Clean up and close interfaces
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
