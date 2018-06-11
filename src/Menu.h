
#ifndef MENU_H
#define MENU_H

void drawNumbers(int buttonId, int textureID);
void zoomLayout(bool zoom, int buttonId);

extern void setMenuLayoutDimensions();
extern int initMenu();
extern void drawMenu();
extern void freeMenu();

#endif
