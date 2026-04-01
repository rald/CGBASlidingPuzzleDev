#include <SDL2/SDL.h>

#include "image.h"

#define GAME_TITLE "Game"
#define SCREEN_WIDTH (image_width)
#define SCREEN_HEIGHT (image_height)

SDL_Color palette[]={
    { 26, 28, 44, 255},
    { 93, 39, 93, 255},
    {177, 62, 83, 255},
    {239,125, 87, 255},
    {255,205,117, 255},
    {167,240,112, 255},
    { 56,183,100, 255},
    { 37,113,121, 255},
    { 41, 54,111, 255},
    { 59, 93,201, 255},
    { 65,166,246, 255},
    {115,239,247, 255},
    {244,244,244, 255},
    {148,176,194, 255},
    { 86,108,134, 255},
    { 51, 60, 87, 255},
};

Uint32 get_pixel(SDL_Surface *surface, int x, int y) {
    int bpp = surface->format->BytesPerPixel;
    // Address of the pixel
    Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;

    switch (bpp) {
        case 1: return *p;
        case 2: return *(Uint16 *)p;
        case 3: // 24-bit (handles endianness)
            if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
                return p[0] << 16 | p[1] << 8 | p[2];
            else
                return p[0] | p[1] << 8 | p[2] << 16;
        case 4: return *(Uint32 *)p;
        default: return 0;
    }
}

int main(void) {

	bool quit=false;

	SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window =    SDL_CreateWindow(GAME_TITLE,
                            SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                            SCREEN_WIDTH, SCREEN_HEIGHT,
                            SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);

    SDL_Renderer *renderer =    SDL_CreateRenderer(window, -1,
                                SDL_RENDERER_ACCELERATED |
                                SDL_RENDERER_TARGETTEXTURE);

   	SDL_Event event;

	SDL_Surface *bmp=SDL_LoadBMP("image.bmp");


	FILE *fp=fopen("image.cvs","w");
	char *hex="0123456789ABCDEF";
	for(int j=0;j<bmp->h;j++) {
		for(int i=0;i<bmp->w;i++) {
			Uint32 k=get_pixel(bmp,i,j);
			Uint8 r, g, b, a;
			SDL_GetRGBA(k, bmp->format, &r, &g, &b, &a);
			SDL_SetRenderDrawColor(renderer,r,g,b,a);
			SDL_RenderDrawPoint(renderer,i,j);

			for(int l=0;l<16;l++) {
				if(r==palette[l].r && g==palette[l].g && b==palette[l].b) {
					fprintf(fp,"%2d,",l);
				}
			}
		}
		fprintf(fp,"\n");
	}
	fprintf(fp,"\n");
	fclose(fp);

	SDL_RenderPresent(renderer);

    while(!quit) {
        while(SDL_PollEvent(&event)) {
            switch(event.type) {
            case SDL_QUIT:
                quit=true;
            case SDL_KEYDOWN:
                switch(event.key.keysym.sym) {
                case SDLK_ESCAPE:
                    quit=true;
                default: break;
                }
            default: break;
            }
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);

	return 0;
}
