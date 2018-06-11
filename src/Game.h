
#ifndef GAME_H
#define GAME_H

extern int initGame();
extern void drawGame();
extern void setOrientationPlane();
extern void freeGame();

int randomInt(int min, int max);
static void drawAllCoins();
static int initCoins();
static void drawOneCoin(int x, int y, int z, int id);
void drawPlane();

#define NB_COINS    300
#define NB_BOMB      50
#define BOMB_ID       0
#define COIN_ID       1

struct Coin
{
    int id;
    float posX;
    float posY;
    float posZ;
    float rotation;
    bool available;
};
typedef struct Coin* pCoin;


#endif
