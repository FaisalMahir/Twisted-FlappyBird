#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <iostream>
#include <cstdlib>
#include <ctime>

int WindowWidth = 800;
int WindowHeight = 600;

float birdY = 300;
float velocity = 0;
float gravity = 0.4;
float jumpStrength = -7;

int birdX = 100;
int birdSize = 40;

int pipeWidth = 80;
int pipeGap = 125;
int pipeSpeed = 4;

struct Pipe {
    int x;
    int gapY;
};

Pipe pipe1;
Pipe pipe2;

char currentKey = 'A';

// Picks a random uppercase letter A–Z
char randomLetter() {
    return 'A' + (rand() % 26);
}

bool checkCollision(Pipe p){
    SDL_Rect bird = {birdX, (int)birdY, birdSize, birdSize };
    SDL_Rect topPipe = {p.x, 0, pipeWidth, p.gapY };
    SDL_Rect bottomPipe = {p.x, p.gapY + pipeGap, pipeWidth, WindowHeight - (p.gapY + pipeGap) };

    return SDL_HasIntersection(&bird, &topPipe) || SDL_HasIntersection(&bird, &bottomPipe);
}

void resetGame(int &score) {
    birdY = 300;
    velocity = 0;
    score = 0;

    pipe1 = {WindowWidth, rand() % (WindowHeight/2) + 100};
    pipe2 = {WindowWidth + WindowWidth/2, rand() % (WindowHeight/2) + 100};

    currentKey = randomLetter();
}

int main() {
    srand(time(NULL));

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL Init error: " << SDL_GetError() << "\n";
        return 1;
    }

    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) {
        std::cout << "IMG_Init Error: " << IMG_GetError() << "\n";
        return 1;
    }

    if (TTF_Init() < 0) {
        std::cout << "TTF Init Error: " << TTF_GetError() << "\n";
        return 1;
    }

    SDL_Window* win = SDL_CreateWindow("Flappy Bird", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WindowWidth, WindowHeight, SDL_WINDOW_SHOWN);
    SDL_Renderer* renderer = SDL_CreateRenderer(win, -1, SDL_RENDERER_ACCELERATED);

    TTF_Font* font = TTF_OpenFont("assets/arial.ttf", 48);
    TTF_Font* smallFont = TTF_OpenFont("assets/arial.ttf", 24);
    if (!font) {
        std::cout << "Failed to load font: " << TTF_GetError() << "\n";
        return 1;
    }

    int score = 0;
    bool running = true;
    bool gameOver = false;
    SDL_Event e;

    resetGame(score);

    SDL_Texture* background = IMG_LoadTexture(renderer, "assets/Background.png");
    if (!background) {
        std::cout << "Failed to load background: " << IMG_GetError() << "\n";
        return 1;
    }

    SDL_Texture* birdTexture = IMG_LoadTexture(renderer, "assets/Pakhi.png");
    if (!birdTexture) {
        std::cout << "Failed to load bird image: " << IMG_GetError() << "\n";
        return 1;
    }

    while (running) {
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) running = false;

            // Jump ONLY if the pressed key == currentKey
            if (!gameOver && e.type == SDL_KEYDOWN) {
                char pressed = SDL_GetKeyName(e.key.keysym.sym)[0];

                if (pressed >= 'a' && pressed <= 'z') pressed -= 32;

                if (pressed == currentKey) {
                    velocity = jumpStrength;
                }
            }

            if (gameOver && e.type == SDL_KEYDOWN && e.key.keysym.sym == SDLK_r) {
                resetGame(score);
                gameOver = false;
            }
        }

        if (!gameOver) {
            velocity += gravity;
            birdY += velocity;

            pipe1.x -= pipeSpeed;
            pipe2.x -= pipeSpeed;

            // When pipe1 passes
            if (pipe1.x + pipeWidth < 0) {
                pipe1.x = WindowWidth;
                pipe1.gapY = rand() % (WindowHeight/2) + 100;
                score++;

                currentKey = randomLetter();
            }

            // When pipe2 passes
            if (pipe2.x + pipeWidth < 0) {
                pipe2.x = WindowWidth;
                pipe2.gapY = rand() % (WindowHeight/2) + 100;
                score++;

                currentKey = randomLetter();
            }

            if (checkCollision(pipe1) || checkCollision(pipe2) || birdY < 0 || birdY + birdSize + 60 > WindowHeight) {
                gameOver = true;
            }
        }

        SDL_RenderCopy(renderer, background, NULL, NULL);

        SDL_Rect birdRect = {birdX, (int)birdY, birdSize, birdSize};
        SDL_RenderCopy(renderer, birdTexture, NULL, &birdRect);

        SDL_SetRenderDrawColor(renderer, 0, 200, 0, 255);
        SDL_Rect p1_top = {pipe1.x, 0, pipeWidth, pipe1.gapY};
        SDL_Rect p1_bottom = {pipe1.x, pipe1.gapY + pipeGap, pipeWidth, WindowHeight - (pipe1.gapY + pipeGap)};
        SDL_RenderFillRect(renderer, &p1_top);
        SDL_RenderFillRect(renderer, &p1_bottom);

        SDL_Rect p2_top = {pipe2.x, 0, pipeWidth, pipe2.gapY};
        SDL_Rect p2_bottom = {pipe2.x, pipe2.gapY + pipeGap, pipeWidth, WindowHeight - (pipe2.gapY + pipeGap)};
        SDL_RenderFillRect(renderer, &p2_top);
        SDL_RenderFillRect(renderer, &p2_bottom);

        // Draw score
        SDL_Color black = {0, 0, 0, 255};
        SDL_Surface* scoreSurf = TTF_RenderText_Solid(font, std::to_string(score).c_str(), black);
        SDL_Texture* scoreTexture = SDL_CreateTextureFromSurface(renderer, scoreSurf);
        SDL_Rect scoreRect = {20, 20, scoreSurf->w, scoreSurf->h};
        SDL_RenderCopy(renderer, scoreTexture, NULL, &scoreRect);
        SDL_FreeSurface(scoreSurf);
        SDL_DestroyTexture(scoreTexture);

        // Draw current key required to jump
        std::string keyText = std::string("Press ") + currentKey;
        SDL_Surface* keySurf = TTF_RenderText_Solid(font, keyText.c_str(), black);
        SDL_Texture* keyTex = SDL_CreateTextureFromSurface(renderer, keySurf);
        SDL_Rect keyRect = {20, 80, keySurf->w, keySurf->h};
        SDL_RenderCopy(renderer, keyTex, NULL, &keyRect);
        SDL_FreeSurface(keySurf);
        SDL_DestroyTexture(keyTex);

        if (gameOver) {
            SDL_Color red = {255, 50, 50, 255};
            SDL_Surface* goSurf = TTF_RenderText_Solid(font, "GAME OVER", red);
            SDL_Texture* goTex = SDL_CreateTextureFromSurface(renderer, goSurf);
            SDL_Rect goRect = {WindowWidth/2 - goSurf->w/2, 170, goSurf->w, goSurf->h};
            SDL_RenderCopy(renderer, goTex, NULL, &goRect);
            SDL_FreeSurface(goSurf);
            SDL_DestroyTexture(goTex);

            SDL_Surface* goSurf2 = TTF_RenderText_Solid(smallFont, "Press the mentioned letter to jump", black);
            SDL_Texture* goTex2 = SDL_CreateTextureFromSurface(renderer, goSurf2);
            SDL_Rect goRect2 = {WindowWidth/2 - goSurf2->w/2, 230, goSurf2->w, goSurf2->h};
            SDL_RenderCopy(renderer, goTex2, NULL, &goRect2);
            SDL_FreeSurface(goSurf2);
            SDL_DestroyTexture(goTex2);

            SDL_Surface* goSurf3 = TTF_RenderText_Solid(smallFont, "The letter changes after every pipe", black);
            SDL_Texture* goTex3 = SDL_CreateTextureFromSurface(renderer, goSurf3);
            SDL_Rect goRect3 = {WindowWidth/2 - goSurf3->w/2, 260, goSurf3->w, goSurf3->h};
            SDL_RenderCopy(renderer, goTex3, NULL, &goRect3);
            SDL_FreeSurface(goSurf3);
            SDL_DestroyTexture(goTex3);

            SDL_Surface* retrySurf = TTF_RenderText_Solid(font, "Press R to Restart", red);
            SDL_Texture* retryTex = SDL_CreateTextureFromSurface(renderer, retrySurf);
            SDL_Rect retryRect = {WindowWidth/2 - retrySurf->w/2, 320, retrySurf->w, retrySurf->h};
            SDL_RenderCopy(renderer, retryTex, NULL, &retryRect);
            SDL_FreeSurface(retrySurf);
            SDL_DestroyTexture(retryTex);
        }

        SDL_RenderPresent(renderer);
        SDL_Delay(27);
    }

    SDL_DestroyTexture(birdTexture);
    SDL_DestroyTexture(background);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(win);
    TTF_CloseFont(font);
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();

    return 0;
}
