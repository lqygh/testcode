#include <SDL.h>

struct coordinate_3d {
	int x;
	int y;
	int z;
};

struct coordinate_2d {
	int x;
	int y;
};

struct triangle {
	struct coordinate_3d v1;
	struct coordinate_3d v2;
	struct coordinate_3d v3;
};

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
	struct coordinate_2d* window_size;
	SDL_Renderer* renderer;
	struct triangle* triangle;
};

void move_triangle(struct triangle* triangle, struct coordinate_3d movement) {
	triangle->v1.x = triangle->v1.x + movement.x;
	triangle->v1.y = triangle->v1.y + movement.y;
	triangle->v1.z = triangle->v1.z + movement.z;
	
	triangle->v2.x = triangle->v2.x + movement.x;
	triangle->v2.y = triangle->v2.y + movement.y;
	triangle->v2.z = triangle->v2.z + movement.z;
	
	triangle->v3.x = triangle->v3.x + movement.x;
	triangle->v3.y = triangle->v3.y + movement.y;
	triangle->v3.z = triangle->v3.z + movement.z;
}

void draw_triangle(SDL_Renderer* renderer, struct triangle* triangle, struct coordinate_2d* window_size) {
	if(window_size->x <= 0 || window_size->y <= 0) {
		printf("width and height must be positive\n");
		return;
	}
	
	int i, j;
	double barycentric[3];
	for(i = 0; i < window_size->x; i++) {
		for(j = 0; j < window_size->y; j++) {
			barycentric[0] = (double)((triangle->v2.y - triangle->v3.y) * (i - triangle->v3.x) + (triangle->v3.x - triangle->v2.x) * (j - triangle->v3.y)) /
			                 ((triangle->v2.y - triangle->v3.y) * (triangle->v1.x - triangle->v3.x) + (triangle->v3.x - triangle->v2.x) * (triangle->v1.y - triangle->v3.y));
			
			barycentric[1] = (double)((triangle->v3.y - triangle->v1.y) * (i - triangle->v3.x) + (triangle->v1.x - triangle->v3.x) * (j - triangle->v3.y)) /
			                 ((triangle->v2.y - triangle->v3.y) * (triangle->v1.x - triangle->v3.x) + (triangle->v3.x - triangle->v2.x) * (triangle->v1.y - triangle->v3.y));
			
			barycentric[2] = 1 - barycentric[0] - barycentric[1];
			
			if(barycentric[0] >= 0 && barycentric[0] <= 1 && barycentric[1] >= 0 && barycentric[1] <= 1 && barycentric[2] >= 0 && barycentric[2] <= 1) {
				if(SDL_SetRenderDrawColor(renderer, barycentric[0] * 255, barycentric[1] * 255, barycentric[2] * 255, 255) < 0) {
					printf("Failed to set renderer color: %s\n", SDL_GetError());
					return;
				}
	
				if(SDL_RenderDrawPoint(renderer, i, j) < 0) {
					printf("Failed to draw point: %s\n", SDL_GetError());
					return;
				}
			}
		}
	}
}

void myupdate(struct draw_state* state) {
	if(state->width <= 0 || state->height <= 0) {
		printf("width and height must be positive\n");
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
	struct coordinate_2d* window_size = args->window_size;
	SDL_Renderer* renderer = args->renderer;
	struct triangle* triangle = args->triangle;
	
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
		
		//draw triangle
		draw_triangle(renderer, triangle, window_size);
		
		//update point
		myupdate(dstate);
		
		//draw point
		mydraw(renderer, dstate);
		
		//update screen
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
	
	struct coordinate_2d window_size = {width, height};
	struct triangle triangle = {{100, 350, 999}, {200, 150, 666}, {300, 350, 333}};
	struct draw_thread_args dtargs = {1, &delay, &i, &dstate, &window_size, renderer, &triangle};
	
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
			} else if(event.type == SDL_KEYDOWN) {
				SDL_KeyboardEvent* ev = &(event.key);
				struct coordinate_3d mov;
				if(ev->keysym.sym == SDLK_UP) {
					mov = (struct coordinate_3d){0, -1, 0};
					move_triangle(&triangle, mov);
				} else if(ev->keysym.sym == SDLK_DOWN) {
					mov = (struct coordinate_3d){0, 1, 0};
					move_triangle(&triangle, mov);
				} else if(ev->keysym.sym == SDLK_LEFT) {
					mov = (struct coordinate_3d){-1, 0, 0};
					move_triangle(&triangle, mov);
				} else if(ev->keysym.sym == SDLK_RIGHT) {
					mov = (struct coordinate_3d){1, 0, 0};
					move_triangle(&triangle, mov);
				} else if(ev->keysym.sym == SDLK_COMMA) {
					mov = (struct coordinate_3d){0, 0, 1};
					move_triangle(&triangle, mov);
				} else if(ev->keysym.sym == SDLK_PERIOD) {
					mov = (struct coordinate_3d){0, 0, -1};
					move_triangle(&triangle, mov);
				} else {
					delay += 1;
				}
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
