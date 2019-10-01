#include <iostream>
#include <vector>
#include <cstdlib>
#include <SDL2/SDL.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_image.h>
using namespace std;

struct point {
  point() = default;
  point(float x, float y): x(x), y(y) {};
  float x = 0;
  float y = 0;
};

typedef vector<point> vpoint;

enum eDirection {STOP=0, LEFT, RIGHT, UP, DOWN};

struct Snake
{
  Snake() = default;
  Snake(float x, float y): mHead(point(x, y)) {};
  void Update(float, float);
  point mHead = point();
  vpoint mTail;
  eDirection mDir = STOP;
  int mWidth = 20;
  int mHeight = 20;
};

class Game
{
public:
  Game() = default;
  // int getWidth() const {return mWidth;}
  // int getHeight() const {return mHeight;}
  // Snake getSnake() const {return mSnake;}
  // point getFruit() const {return mFruit;}
  // int getScore() const {return mScore;}

  bool Initial();
  void Loop();
  void Close();
private:
  void ProcessInput();
  void UpdateGame();
  void GenerateOutput();

  bool mIsRunning = true;
  bool mPause     = false;
  int mWidth      = 1024;
  int mHeight     = 768;
  Snake mSnake;
  point mFruit    = point();

  int mScore = 0;

  SDL_Window *mWindow;

  SDL_Renderer *mRenderer;

  Uint32 mTickCount;

  int mFPS = 30;
};

bool Game::Initial()
{
  // SDL initialisations
  int sdlresult = SDL_Init(SDL_INIT_VIDEO);
  if (sdlresult != 0) {
    SDL_Log("Unable to initialize: %s", SDL_GetError());
    return false;
  }

  mWindow = SDL_CreateWindow("Snakes Game",
                             SDL_WINDOWPOS_CENTERED,
                             SDL_WINDOWPOS_CENTERED,
                             mWidth, mHeight, SDL_WINDOW_RESIZABLE);

  if (!mWindow) {
    SDL_Log("Fail to create window: %s", SDL_GetError());
    return false;
  }

  mRenderer = SDL_CreateRenderer(mWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

  if (!mRenderer) {
    SDL_Log("Fail to create renderer: %s", SDL_GetError());
    return false;
  }

  // Game initialisations
  mSnake = Snake((float)(mWidth) / 2.0f, (float)(mHeight) / 2.0f);
  mFruit = point((float)(rand() % mWidth) + 1.0f, (float)(rand() % mWidth) + 1.0f);

  return true;
}

void Game::Loop()
{
  while (mIsRunning) {
    ProcessInput();
    if (!mPause) {
      UpdateGame();
      GenerateOutput();
    }
  }
}

void Game::Close()
{
  SDL_DestroyRenderer(mRenderer);
  SDL_DestroyWindow(mWindow);
  SDL_Quit();
}

void Game::ProcessInput()
{
  SDL_Event event;

  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_QUIT:
      mIsRunning = false;
      break;
    default:
      break;
    }

    const Uint8 *state = SDL_GetKeyboardState(nullptr);

    if (state[SDL_SCANCODE_ESCAPE]) 
      mIsRunning = false;
    else if (state[SDL_SCANCODE_A]) 
      mIsRunning = false;
    else if (state[SDL_SCANCODE_J]
             && mSnake.mDir != UP)
      mSnake.mDir = DOWN;
    else if (state[SDL_SCANCODE_K]
             && mSnake.mDir != DOWN)
      mSnake.mDir = UP;
    else if (state[SDL_SCANCODE_H]
             && mSnake.mDir != RIGHT)
      mSnake.mDir = LEFT;
    else if (state[SDL_SCANCODE_L]
             && mSnake.mDir != LEFT)
      mSnake.mDir = RIGHT;
    else if (state[SDL_SCANCODE_S])
      mSnake.mDir = STOP;
  }
}

void Snake::Update(float speed, float delta)
{
  // Tail

  if (mTail.begin() != mTail.end()) {
    for (vector<point>::iterator it = --mTail.end(); it != mTail.begin(); --it) {
      vector<point>::iterator prev = --it;
      ++it;
      it->x = prev->x;
      it->y = prev->y;
    }
    (mTail.begin())->x = mHead.x;
    (mTail.begin())->y = mHead.y;
  }

  // Head
  switch (mDir) {
  case STOP:
    break;
  case UP:
    mHead.y -= speed * delta;
    break;
  case DOWN:
    mHead.y += speed * delta;
    break;
  case LEFT:
    mHead.x -= speed * delta;
    break;
  case RIGHT:
    mHead.x += speed * delta;
    break;
  default:
    break;
  }
  
}

void Game::UpdateGame()
{
  // Frame limiting to 30FPS.
  while (!SDL_TICKS_PASSED(SDL_GetTicks(), mTickCount + 1000 / mFPS)) {}
  
  float deltaTime = (SDL_GetTicks() - mTickCount) / 1000.0f;
  const float max_delta_time = 0.05f;

  deltaTime = deltaTime > max_delta_time ? max_delta_time : deltaTime;

  mTickCount = SDL_GetTicks();

  const float snake_speed = 300.0f;

  mSnake.Update(snake_speed, deltaTime);

  // Screen boundaries
  if (mSnake.mHead.x < mSnake.mWidth / 2.0f) 
    mIsRunning = false;
  else if (mSnake.mHead.x > mWidth - mSnake.mWidth / 2.0f)
    mIsRunning = false;
  else if (mSnake.mHead.y < mSnake.mHeight / 2.0f)
    mIsRunning = false;
  else if (mSnake.mHead.y > mHeight - mSnake.mHeight / 2.0f)
    mIsRunning = false;

  // eat fruit
  if (mSnake.mHead.x - mFruit.x > -mSnake.mWidth &&
      mSnake.mHead.x - mFruit.x < mSnake.mWidth &&
      mSnake.mHead.y - mFruit.y < mSnake.mHeight &&
      mSnake.mHead.y - mFruit.y > -mSnake.mHeight) {
    ++mScore;
    mFruit = point((float)(rand() % mWidth), (float)(rand() % mHeight));
    mSnake.mTail.push_back(point(mFruit.x, mFruit.y));
  }

  // eat itself
  for (vector<point>::iterator it = mSnake.mTail.begin(); it != mSnake.mTail.end(); ++it) {
    if (it->x == mSnake.mHead.x && it->y == mSnake.mHead.y)
      mIsRunning = false;
  }
}

void Game::GenerateOutput()
{
  SDL_SetRenderDrawColor(mRenderer, 0, 0, 0, 100);
  SDL_RenderClear(mRenderer);

  SDL_Rect head;
  head.x = static_cast<int>(mSnake.mHead.x - mSnake.mWidth / 2.0f);
  head.y = static_cast<int>(mSnake.mHead.y - mSnake.mHeight / 2.0f);
  head.w = mSnake.mWidth;
  head.h = mSnake.mHeight;

  SDL_SetRenderDrawColor(mRenderer, 0, 255, 0, 255);

  SDL_RenderFillRect(mRenderer, &head);

  vector<SDL_Rect> tail_rects(mSnake.mTail.size(), SDL_Rect());

  for (vector<point>::size_type i = 0; i < mSnake.mTail.size(); ++i) {
    vector<SDL_Rect>::size_type j = i;
    tail_rects[j].x = static_cast<int>(mSnake.mTail[i].x - mSnake.mWidth / 2.0f);
    tail_rects[j].y = static_cast<int>(mSnake.mTail[i].y - mSnake.mHeight / 2.0f);
    tail_rects[j].w = mSnake.mWidth;
    tail_rects[j].h = mSnake.mHeight;

    SDL_SetRenderDrawColor(mRenderer, 255, 0, 0, 255);
    
    SDL_RenderFillRect(mRenderer, &(tail_rects[j]));
  }

  SDL_Rect fruit;
  fruit.x = static_cast<int>(mFruit.x - mSnake.mWidth / 2.0f);
  fruit.y = static_cast<int>(mFruit.y - mSnake.mHeight / 2.0f);
  fruit.w = mSnake.mWidth;
  fruit.h = mSnake.mHeight;

  // if (tail_rects.size() > 1 && tail_rects[1].y > 0 && tail_rects[1].y < mHeight)
  SDL_SetRenderDrawColor(mRenderer, 0, 0, 255, 255);
  // else
  // SDL_SetRenderDrawColor(mRenderer, 0, 255, 0, 255);

  SDL_RenderFillRect(mRenderer, &fruit);
  
  SDL_RenderPresent(mRenderer);
}

int main()
{
  Game game;

  bool success = game.Initial();

  if (success) 
    game.Loop();


  game.Close();
  
  return 0;
}

  // mWindow = SDL_CreateWindow("Snakes",
  //                            SDL_WINDOWPOS_CENTERED,
  //                            SDL_WINDOWPOS_CENTERED,
  //                            mWidth, mHeight, 0);
  // if (!mWindow) {
  //   SDL_Log("Fail to create window: %s", SDL_GetError());
  //   return false;
  // }
  // mRenderer = 

  // archive


  // void setup () {
  //   initscr();
  //   clear();
  //   noecho();
  //   cbreak;
  //   curs_set(0);

  //   dead = false;

  //   dir = STOP;

  //   player->x = width / 2;
  //   player->y = height / 2;
  //   fruit->x  = (rand() % width) + 1;
  //   fruit->y  = (rand() % height) + 1;

  //   score = 0;
  // }

  // void draw () {
  //   clear();

  //   for (int i = 0; i < width + 2; ++i) {
  //     mvprintw(0, i, "+");
  //   }

  //   for (int i = 0; i < height + 2; ++i) {
  //     for (int j = 0; j < width + 2; ++j) {
  //       if (i == 0 || i == width + 1) {
  //         mvprintw(i, j, "+");
  //       } else if (j == 0 || j == height + 1) {
  //         mvprintw(i, j, "+");
  //       } else if (i == player->y && j == player-> x) {
  //         mvprintw(i, j, "O");
  //       } else if (i == fruit-> y && j == fruit-> x) {
  //         mvprintw(i, j, "@");
  //       }
  //     }
  //   }

  //   for (vector<point>::iterator it = tail.begin(); it != tail.end(); ++it)
  //     mvprintw(it->y, it->x, "o");


  //   mvprintw(height + 2, 0, "Score: %d", score);

  //   refresh();
  // }

  // void input () {
  //   keypad(stdscr, TRUE);
  //   halfdelay(1);

  //   int c = getch();

  //   switch (c) {
  //   case KEY_LEFT:
  //     if (dir == RIGHT)
  //       break;
  //     dir = LEFT;
  //     break;
  //   case KEY_RIGHT:
  //     if (dir == LEFT)
  //       break;
  //     dir = RIGHT;
  //     break;
  //   case KEY_UP:
  //     if (dir == DOWN)
  //       break;
  //     dir = UP;
  //     break;
  //   case KEY_DOWN:
  //     if (dir == UP)
  //       break;
  //     dir = DOWN;
  //     break;
  //   case 113:
  //     dead = true;
  //     break;
  //   default:
  //     break;
  //   }
  // }

  // void update () {
  //   if (tail.begin() != tail.end()) {
  //     for (vector<point>::iterator it = --(tail.end()); it != tail.begin(); --it) {
  //       vector<point>::iterator prev = --it;
  //       ++it;
  //       it->x = prev->x;
  //       it->y = prev->y;
  //     }

  //     (tail.begin())->x = player->x;
  //     (tail.begin())->y = player->y;
  //   }

  //   switch (dir) {
  //   case LEFT:
  //     --(player->x);
  //     break;
  //   case RIGHT:
  //     ++(player->x);
  //     break;
  //   case UP:
  //     --(player->y);
  //     break;
  //   case DOWN:
  //     ++(player->y);
  //     break;
  //   default:
  //     break;
  //   }

  //   if (player->x > width || player->x < 1 || player->y < 1 || player->y > height) {
  //     dead = true;
  //   }

  //   if (player->x == fruit->x && player->y == fruit->y) {
  //     ppoint temp = new point();
  //     temp->x = fruit->x;
  //     temp->y = fruit->y;
  //     tail.push_back(*temp);
  //     ++score;
  //     fruit->x = (rand() % width) + 1;
  //     fruit->y = (rand() % height) + 1;
  //   }
  // }

  // int main() {
  //   setup();

  //   while (!dead) {
  //     draw();
  //     input();
  //     update();
  //   }

  //   getch();
  //   endwin();

  //   delete player;
  //   delete fruit;

  //   return 0;
  // }

  // bool dead;

  // const int width  = 20;
  // const int height = 20;


  // typedef point * ppoint;

  // ppoint player = new point();
  // ppoint fruit = new point();

  // vector<point> tail;

  // int score;

  // snakeDirection dir;
