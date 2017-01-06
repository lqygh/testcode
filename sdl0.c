#include <SDL.h>

struct coordinate_3df {
	double x;
	double y;
	double z;
	double w;
};

struct coordinate_2d {
	int x;
	int y;
};

struct trianglef {
	struct coordinate_3df v1;
	struct coordinate_3df v2;
	struct coordinate_3df v3;
};

struct triangles {
	struct coordinate_3df* vertices;
	int num_vertices;
	int** triangle_vertices_indexes;
	int num_triangles;
};

struct bouncing_point_state {
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
	struct bouncing_point_state* bpstate;
	struct coordinate_2d* window_size;
	SDL_Renderer* renderer;
	struct trianglef* triangle;
};

void translate(struct coordinate_3df* input, struct coordinate_3df* output, struct coordinate_3df* movement) {
	output->x = input->x + input->w * movement->x;
	output->y = input->y + input->w * movement->y;
	output->z = input->z + input->w * movement->z;
}

void rotate(struct coordinate_3df* input, struct coordinate_3df* output, int axis, double sin, double cos) {
	double i[3] = {input->x, input->y, input->z};
	double o[3];
	
	if(axis == 0) {
		//rotate about x axis
		o[0] = i[0];
		o[1] = i[1] * cos - i[2] * sin;
		o[2] = i[1] * sin + i[2] * cos;
	} else if(axis == 1) {
		//rotate about y axis
		o[0] = i[0] * cos + i[2] * sin;
		o[1] = i[1];
		o[2] = i[0] * (-sin) + i[2] * cos;
	} else if(axis == 2) {
		//rotate about z axis
		o[0] = i[0] * cos - i[1] * sin;
		o[1] = i[0] * sin + i[1] * cos;
		o[2] = i[2];
	} else {
		return;
	}
	
	output->x = o[0];
	output->y = o[1];
	output->z = o[2];
}

void scale(struct coordinate_3df* input, struct coordinate_3df* output, double factor) {
	output->x = (input->x) * factor;
	output->y = (input->y) * factor;
	output->z = (input->z) * factor;
}

void perspective(struct coordinate_3df* input, struct coordinate_3df* output, double amount) {
	output->w = (input->w) + amount;
}

void translate_triangle(struct trianglef* triangle, struct coordinate_3df movement) {
	translate(&(triangle->v1), &(triangle->v1), &movement);
	translate(&(triangle->v2), &(triangle->v2), &movement);
	translate(&(triangle->v3), &(triangle->v3), &movement);
}

void rotate_triangle(struct trianglef* triangle, int axis, double sin, double cos) {	
	rotate(&(triangle->v1), &(triangle->v1), axis, sin, cos);
	rotate(&(triangle->v2), &(triangle->v2), axis, sin, cos);
	rotate(&(triangle->v3), &(triangle->v3), axis, sin, cos);
}

void scale_triangle(struct trianglef* triangle, double factor) {
	scale(&(triangle->v1), &(triangle->v1), factor);
	scale(&(triangle->v2), &(triangle->v2), factor);
	scale(&(triangle->v3), &(triangle->v3), factor);
}

void perspective_triangle(struct trianglef* triangle, double amount) {
	perspective(&(triangle->v1), &(triangle->v1), amount);
	perspective(&(triangle->v2), &(triangle->v2), amount);
	perspective(&(triangle->v3), &(triangle->v3), amount);
}

void translate_vertices(struct coordinate_3df* vertices, int number, struct coordinate_3df movement) {
	int i;
	for(i = 0; i < number; i++) {
		translate(&(vertices[i]), &(vertices[i]), &movement);
	}
}

void rotate_vertices(struct coordinate_3df* vertices, int number, int axis, double sin, double cos) {
	int i;
	for(i = 0; i < number; i++) {
		rotate(&(vertices[i]), &(vertices[i]), axis, sin, cos);
	}
}

void scale_vertices(struct coordinate_3df* vertices, int number, double factor) {
	int i;
	for(i = 0; i < number; i++) {
		scale(&(vertices[i]), &(vertices[i]), factor);
	}
}

void perspective_vertices(struct coordinate_3df* vertices, int number, double amount) {
	int i;
	for(i = 0; i < number; i++) {
		perspective(&(vertices[i]), &(vertices[i]), amount);
	}
}

void draw_triangle(SDL_Renderer* renderer, struct trianglef* triangle, struct coordinate_2d* window_size) {
	if(window_size->x <= 0 || window_size->y <= 0) {
		printf("width and height must be positive\n");
		return;
	}
	
	if(triangle->v1.w == 0 || triangle->v2.w == 0 || triangle->v3.w == 0) {
		return;
	}
	
	struct trianglef tri = {0};
	tri.v1.x = (triangle->v1.x) / (triangle->v1.w);
	tri.v1.y = (triangle->v1.y) / (triangle->v1.w);
	tri.v1.z = (triangle->v1.z) / (triangle->v1.w);
	tri.v2.x = (triangle->v2.x) / (triangle->v2.w);
	tri.v2.y = (triangle->v2.y) / (triangle->v2.w);
	tri.v2.z = (triangle->v2.z) / (triangle->v2.w);
	tri.v3.x = (triangle->v3.x) / (triangle->v3.w);
	tri.v3.y = (triangle->v3.y) / (triangle->v3.w);
	tri.v3.z = (triangle->v3.z) / (triangle->v3.w);
	
	double min_x = window_size->x;
	double max_x = 0;
	double min_y = window_size->y;
	double max_y = 0;
	if(tri.v1.x < min_x) min_x = tri.v1.x;
	if(tri.v2.x < min_x) min_x = tri.v2.x;
	if(tri.v3.x < min_x) min_x = tri.v3.x;
	if(tri.v1.x > max_x) max_x = tri.v1.x;
	if(tri.v2.x > max_x) max_x = tri.v2.x;
	if(tri.v3.x > max_x) max_x = tri.v3.x;
	if(tri.v1.y < min_y) min_y = tri.v1.y;
	if(tri.v2.y < min_y) min_y = tri.v2.y;
	if(tri.v3.y < min_y) min_y = tri.v3.y;
	if(tri.v1.y > max_y) max_y = tri.v1.y;
	if(tri.v2.y > max_y) max_y = tri.v2.y;
	if(tri.v3.y > max_y) max_y = tri.v3.y;
	
	int min_x_int = (int)min_x;
	int max_x_int = (int)max_x;
	int min_y_int = (int)min_y;
	int max_y_int = (int)max_y;
	if(min_x_int < 0) min_x_int = 0;
	if(max_x_int > window_size->x) max_x_int = window_size->x;
	if(min_y_int < 0) min_y_int = 0;
	if(max_y_int > window_size->y) max_y_int = window_size->y;
	
	int i, j;
	for(j = min_y_int; j < max_y_int; j++) {
		for(i = min_x_int; i < max_x_int; i++) {
			double barycentric[3];
			int drawn = 0;
			
			barycentric[0] = ((tri.v2.y - tri.v3.y) * (i - tri.v3.x) + (tri.v3.x - tri.v2.x) * (j - tri.v3.y)) /
			                 ((tri.v2.y - tri.v3.y) * (tri.v1.x - tri.v3.x) + (tri.v3.x - tri.v2.x) * (tri.v1.y - tri.v3.y));
			
			barycentric[1] = ((tri.v3.y - tri.v1.y) * (i - tri.v3.x) + (tri.v1.x - tri.v3.x) * (j - tri.v3.y)) /
			                 ((tri.v2.y - tri.v3.y) * (tri.v1.x - tri.v3.x) + (tri.v3.x - tri.v2.x) * (tri.v1.y - tri.v3.y));
			
			barycentric[2] = 1 - barycentric[0] - barycentric[1];
			
			if(barycentric[0] >= 0 && barycentric[0] <= 1 && barycentric[1] >= 0 && barycentric[1] <= 1 && barycentric[2] >= 0 && barycentric[2] <= 1) {
				if(SDL_SetRenderDrawColor(renderer, barycentric[0] * 255, barycentric[1] * 255, barycentric[2] * 255, 255) < 0) {
					printf("Failed to set renderer color: %s\n", SDL_GetError());
					return;
				}
				
				/* double z_coord = barycentric[0] * tri.v1.z + barycentric[1] * tri.v2.z + barycentric[2] * tri.v3.z;
				double z_fac = 1.0;
				if(z_coord > 1000.0) {
					z_fac = 0;
				} else if(z_coord < 0) {
					z_fac = 1.0;
				} else {
					z_fac = 1.0 - (z_coord / 1000.0);
				}
				
				if(SDL_SetRenderDrawColor(renderer, barycentric[0] * z_fac * 255, barycentric[1] * z_fac * 255, barycentric[2] * z_fac * 255, 255) < 0) {
					printf("Failed to set renderer color: %s\n", SDL_GetError());
					return;
				} */
				
				if(SDL_RenderDrawPoint(renderer, i, j) < 0) {
					printf("Failed to draw point: %s\n", SDL_GetError());
					return;
				}
				
				drawn = 1;
			} else if(drawn == 1) {
				break;
			}
		}
	}
}

void update_bouncing_point(struct bouncing_point_state* state) {
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

void draw_bouncing_point(SDL_Renderer* renderer, struct bouncing_point_state* state) {
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
	struct bouncing_point_state* bpstate = args->bpstate;
	struct coordinate_2d* window_size = args->window_size;
	SDL_Renderer* renderer = args->renderer;
	struct trianglef* triangle = args->triangle;
	
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
		update_bouncing_point(bpstate);
		
		//draw point
		draw_bouncing_point(renderer, bpstate);
		
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
	struct bouncing_point_state bpstate = {width, height, rand()%width, rand()%height, (rand()%15)+1, (rand()%15)+1};
	
	struct coordinate_2d window_size = {width, height};
	struct trianglef triangle = {{100, 250, 0, 1}, {200, 200, 0, 1}, {300, 250, 0, 1}};
	struct draw_thread_args dtargs = {1, &delay, &i, &bpstate, &window_size, renderer, &triangle};
	
	SDL_Thread* dtthread = SDL_CreateThread(draw_thread, "draw_thread", &dtargs);
	if(dtthread == NULL) {
		printf("Failed to create draw thread: %s\n", SDL_GetError());
		return 1;
	}
	
	double sin2 = 0.0348994967;
	double cos2 = 0.99939082701;
	double sinm2 = -0.0348994967;
	double cosm2 = 0.99939082701;
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
				if(ev->keysym.sym == SDLK_UP) {
					translate_triangle(&triangle, (struct coordinate_3df){0, -10, 0, 0});
				} else if(ev->keysym.sym == SDLK_DOWN) {
					translate_triangle(&triangle, (struct coordinate_3df){0, 10, 0, 0});
				} else if(ev->keysym.sym == SDLK_LEFT) {
					translate_triangle(&triangle, (struct coordinate_3df){-10, 0, 0, 0});
				} else if(ev->keysym.sym == SDLK_RIGHT) {
					translate_triangle(&triangle, (struct coordinate_3df){10, 0, 0, 0});
				} else if(ev->keysym.sym == SDLK_COMMA) {
					translate_triangle(&triangle, (struct coordinate_3df){0, 0, -10, 0});
				} else if(ev->keysym.sym == SDLK_PERIOD) {
					translate_triangle(&triangle, (struct coordinate_3df){0, 0, 10, 0});
				} else if(ev->keysym.sym == SDLK_1) {
					rotate_triangle(&triangle, 0, sin2, cos2);
				} else if(ev->keysym.sym == SDLK_2) {
					rotate_triangle(&triangle, 0, sinm2, cosm2);
				} else if(ev->keysym.sym == SDLK_3) {
					rotate_triangle(&triangle, 1, sin2, cos2);
				} else if(ev->keysym.sym == SDLK_4) {
					rotate_triangle(&triangle, 1, sinm2, cosm2);
				} else if(ev->keysym.sym == SDLK_5) {
					rotate_triangle(&triangle, 2, sin2, cos2);
				} else if(ev->keysym.sym == SDLK_6) {
					rotate_triangle(&triangle, 2, sinm2, cosm2);
				} else if(ev->keysym.sym == SDLK_w) {
					perspective_triangle(&triangle, -0.01);
				} else if(ev->keysym.sym == SDLK_s) {
					perspective_triangle(&triangle, 0.01);
				} else if(ev->keysym.sym == SDLK_e) {
					scale_triangle(&triangle, 1.01);
				} else if(ev->keysym.sym == SDLK_d) {
					scale_triangle(&triangle, 0.990099);
				} else if(ev->keysym.sym == SDLK_r) {
					triangle = (struct trianglef){{100, 250, 0, 1}, {200, 200, 0, 1}, {300, 250, 0, 1}};
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
