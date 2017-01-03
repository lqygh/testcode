#include <SDL.h>

struct draw_state {
	int width;
	int height;
	int pos_x;
	int pos_y;
	int v_x;
	int v_y;
};

struct draw_thread_args {
	int should_run;
	Uint8* delay;
	Uint64* framecount;
	struct draw_state* dstate;
	SDL_Renderer* renderer;
};

void myupdate(struct draw_state* state) {
	if(state->width <= 0 || state->height <= 0) {
		fprintf(stderr, "width and height must be positive\n");
		return;
	}
	
	if(state->pos_x < 0) {
		state->pos_x = 0;
		if(state->v_x < 0) {
			state->v_x = 0 - (state->v_x);
		}
	} else if(state->pos_x > state->width) {
		state->pos_x = state->width;
		if(state->v_x > 0) {
			state->v_x = 0 - (state->v_x);
		}
	}
	
	if(state->pos_y < 0) {
		state->pos_y = 0;
		if(state->v_y < 0) {
			state->v_y = 0 - (state->v_y);
		}
	} else if(state->pos_y > state->height) {
		state->pos_y = state->height;
		if(state->v_y > 0) {
			state->v_y = 0 - (state->v_y);
		}
	}
	
	state->pos_x = (state->pos_x) + (state->v_x);
	state->pos_y = (state->pos_y) + (state->v_y);
}

void mydraw(SDL_Renderer* renderer, struct draw_state* state) {
	if(SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255) < 0) {
		printf("Failed to set renderer color: %s\n", SDL_GetError());
		return;
	}
	
	if(SDL_RenderDrawPoint(renderer, state->pos_x, state->pos_y) < 0) {
		printf("Failed to draw point: %s\n", SDL_GetError());
		return;
	}
}

int draw_thread(void* arg) {
	struct draw_thread_args* args = (struct draw_thread_args*)arg;
	Uint8* delay = args->delay;
	Uint64* framecount = args->framecount;
	struct draw_state* dstate = args->dstate;
	SDL_Renderer* renderer = args->renderer;
	
	
	args->should_run = 1;
	while(args->should_run) {
		if(SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255) < 0) {
			printf("Failed to set renderer color: %s\n", SDL_GetError());
			args->should_run = 0;
			return -1;
		}
		
		if(SDL_RenderClear(renderer) < 0) {
			printf("Failed to clear renderer: %s\n", SDL_GetError());
			args->should_run = 0;
			return -1;
		}
		
		//update here
		myupdate(dstate);
		
		//draw here
		mydraw(renderer, dstate);
		
		SDL_RenderPresent(renderer);
		
		(*framecount)++;
		
		SDL_Delay((*delay));
	}
	
	args->should_run = 0;
	return 0;
}

int main(int argc, char* argv[]) {
	int width = 640;
	int height = 480;
	
	if(argc == 3) {
		int new_width = atoi(argv[1]);
		int new_height = atoi(argv[2]);
		if(new_width > 0 && new_height > 0) {
			width = new_width;
			height = new_height;
		}
	}
	
	printf("Amount of RAM in MB: %d\n", SDL_GetSystemRAM());
	printf("Number of logical CPU cores: %d\n", SDL_GetCPUCount());

	if(SDL_Init(SDL_INIT_VIDEO) != 0) {
		printf("Failed to init SDL: %s\n", SDL_GetError());
		return 1;
	}
	
	SDL_Window* window;
	window = SDL_CreateWindow("My Title", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, 0);
	if(window == NULL) {
		printf("Failed to create window: %s\n", SDL_GetError());
		return 1;
	}
	
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, 0);
	if(renderer == NULL) {
		printf("Failed to create renderer: %s\n", SDL_GetError());
		return 1;
	}
	
	Uint64 i = 0;
	Uint8 delay = 0;
	srand(SDL_GetTicks());
	struct draw_state dstate = {width, height, rand()%width, rand()%height, (rand()%15)+1, (rand()%15)+1};
	
	struct draw_thread_args dtargs = {1, &delay, &i, &dstate, renderer};
	SDL_Thread* dtthread = SDL_CreateThread(draw_thread, "draw_thread", &dtargs);
	if(dtthread == NULL) {
		printf("Failed to create draw thread: %s\n", SDL_GetError());
		return 1;
	}
	
	SDL_Event event;
	while(dtargs.should_run) {
		if(SDL_WaitEvent(&event) == 1) {
			if(event.type == SDL_QUIT) {
				printf("\nSDL_QUIT event occurred\n");
				break;
			} else if(event.type == SDL_MOUSEWHEEL) {
				SDL_MouseWheelEvent* ev = &(event.wheel);
				if(ev->y > 0) {
					delay += 1;
				} else if(ev->y < 0) {
					delay -= 1;
				}
			} else if(event.type == SDL_KEYUP) {
				delay += 1;
			}
			
			printf("\r                                        ");
			printf("\rEvent type: %d, delay: %u ms", event.type, delay);
			fflush(stdout);
		} else {
			printf("\nFailed to wait event: %s\n", SDL_GetError());
			return 1;
		}
	}
	
	printf("Exit in 1 second\n");
	SDL_Delay(1000);
	
	dtargs.should_run = 0;
	SDL_WaitThread(dtthread, NULL);
	printf("Draw thread ended\n");
	
	printf("Rendered %lu frames\n", i);
	
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}
