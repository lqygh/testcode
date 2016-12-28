#include <SDL.h>

int main(int argc, char* argv[]) {
	printf("Number of logical CPU cores: %d\n", SDL_GetCPUCount());

	if(SDL_Init(SDL_INIT_VIDEO) != 0) {
		printf("Failed to init SDL: %s\n", SDL_GetError());
		return 1;
	}
	
	SDL_Window* window;
	window = SDL_CreateWindow("My Title", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 640, 480, 0);
	if(window == NULL) {
		printf("Failed to create window: %s\n", SDL_GetError());
		return 1;
	}
	
	SDL_Event event;
	while(1) {
		if(SDL_WaitEvent(&event) == 1) {
			printf("Event type %d\n", event.type);
			if(event.type == SDL_QUIT) {
				printf("SDL_QUIT event occurred\n");
				break;
			}
		} else {
			printf("Failed to wait event: %s\n", SDL_GetError());
			return 1;
		}
	}
	
	printf("Exit in 3 seconds\n");
	SDL_Delay(3000);
	
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
