
#include <SDL_image.h>
#include "glimage.h"

int 
FlipSurfaceVertically(SDL_Surface *surface) {

  char *a, *b;
  unsigned int size;
  char *data = NULL;

  size = surface->pitch;
  data = (char*)malloc(size);

  if(data == NULL) {
    return 0;
  }

  a = (char*)surface->pixels;
  b = a + size * (surface->h - 1);

  for (; a < b; a+=size, b-=size)  {
    memcpy(data, a, size);
    memcpy(a, b, size);
    memcpy(b, data, size);
  }

  free(data);

  return 1;
}


extern
GLuint glimageLoadAndBind (const char * fname) {

  SDL_Surface * texSurface = NULL;  
  SDL_RWops *io = NULL;
  GLuint tex = 0;
  
  int flags= IMG_INIT_JPG | IMG_INIT_PNG;
  int initted= IMG_Init(flags);
  int sign = (SDL_BYTEORDER == SDL_BIG_ENDIAN) ? 1 : -1;

  if((initted & flags) != flags) {
    printf("IMG_Init: Failed to init required jpg and png support!\n");
    printf("IMG_Init: %s\n", IMG_GetError());
    return 0;
  }

  io = SDL_RWFromFile(fname, "rb");

  if((texSurface = IMG_Load(fname)) == NULL ) {
    fprintf(stderr, "Impossible d'ouvrir le fichier : %s\n", IMG_GetError());
    return 0;
  }

  if ((IMG_isJPG(io)) || (IMG_isPNG(io)))
    FlipSurfaceVertically(texSurface);
  
  IMG_Quit();

  
  glGenTextures(1, &tex);
  
  glBindTexture(GL_TEXTURE_2D, tex); 
  glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  if (texSurface->format->BytesPerPixel == 3){
	if (sign * texSurface->format->Rshift > sign * texSurface->format->Bshift)
	  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texSurface->w, texSurface->h, 0, GL_RGB, GL_UNSIGNED_BYTE, texSurface->pixels);
	else 
	  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texSurface->w, texSurface->h, 0, GL_BGR, GL_UNSIGNED_BYTE, texSurface->pixels);
  }

  if (texSurface->format->BytesPerPixel == 4){
	if (sign * texSurface->format->Rshift > sign * texSurface->format->Bshift)
	  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texSurface->w, texSurface->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, texSurface->pixels);
	else 
	  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texSurface->w, texSurface->h, 0, GL_BGRA, GL_UNSIGNED_BYTE, texSurface->pixels);
  }


  SDL_FreeSurface(texSurface);
  return tex;
}


