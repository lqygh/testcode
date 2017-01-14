#include <SDL.h>
#include "matrix/matrix.h"

struct obj_3d {
	struct vector3* vertices;
	int num_vertices;
	int* triangle_vertices_indices;
	int num_triangles;
};

struct obj_3di {
	struct vector3i* vertices;
	int num_vertices;
	int* triangle_vertices_indices;
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
	SDL_Renderer* renderer;
	int window_width;
	int window_height;
	struct bouncing_point_state* bpstate;
	struct obj_3di* triangles;
	double* zbuffer;
};

/* void translate(struct coordinate_3d* input, struct coordinate_3d* output, struct coordinate_3d* movement) {
	output->x = input->x + input->w * movement->x;
	output->y = input->y + input->w * movement->y;
	output->z = input->z + input->w * movement->z;
}

void rotate(struct coordinate_3d* input, struct coordinate_3d* output, int axis, double sin, double cos) {
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

void scale(struct coordinate_3d* input, struct coordinate_3d* output, double factor) {
	output->x = (input->x) * factor;
	output->y = (input->y) * factor;
	output->z = (input->z) * factor;
}

void perspective(struct coordinate_3d* input, struct coordinate_3d* output, double amount) {
	output->w = (input->w) + amount;
}

void translate_triangle(struct triangle* triangle, struct coordinate_3d movement) {
	translate(&(triangle->v1), &(triangle->v1), &movement);
	translate(&(triangle->v2), &(triangle->v2), &movement);
	translate(&(triangle->v3), &(triangle->v3), &movement);
}

void rotate_triangle(struct triangle* triangle, int axis, double sin, double cos) {	
	rotate(&(triangle->v1), &(triangle->v1), axis, sin, cos);
	rotate(&(triangle->v2), &(triangle->v2), axis, sin, cos);
	rotate(&(triangle->v3), &(triangle->v3), axis, sin, cos);
}

void scale_triangle(struct triangle* triangle, double factor) {
	scale(&(triangle->v1), &(triangle->v1), factor);
	scale(&(triangle->v2), &(triangle->v2), factor);
	scale(&(triangle->v3), &(triangle->v3), factor);
}

void perspective_triangle(struct triangle* triangle, double amount) {
	perspective(&(triangle->v1), &(triangle->v1), amount);
	perspective(&(triangle->v2), &(triangle->v2), amount);
	perspective(&(triangle->v3), &(triangle->v3), amount);
}

void translate_vertices(struct coordinate_3d* vertices, int number, struct coordinate_3d movement) {
	int i;
	for(i = 0; i < number; i++) {
		translate(&(vertices[i]), &(vertices[i]), &movement);
	}
}

void rotate_vertices(struct coordinate_3d* vertices, int number, int axis, double sin, double cos) {
	int i;
	for(i = 0; i < number; i++) {
		rotate(&(vertices[i]), &(vertices[i]), axis, sin, cos);
	}
}

void scale_vertices(struct coordinate_3d* vertices, int number, double factor) {
	int i;
	for(i = 0; i < number; i++) {
		scale(&(vertices[i]), &(vertices[i]), factor);
	}
}

void perspective_vertices(struct coordinate_3d* vertices, int number, double amount) {
	int i;
	for(i = 0; i < number; i++) {
		perspective(&(vertices[i]), &(vertices[i]), amount);
	}
} */

void print_vector3(struct vector3* vec) {
	printf("%f %f %f\n", vec->x, vec->y, vec->z);
}

void print_vector3i(struct vector3i* vec) {
	printf("%d %d %f\n", vec->x, vec->y, vec->z);
}

void print_matrix4(struct matrix4* mat) {
	unsigned char i, j;
	for(i = 0; i < 4; i++) {
		for(j = 0; j < 4; j++) {
			printf("%f ", mat->value[i][j]);
		}
		putchar('\n');
	}
}

void obj_to_raster(struct vector3* world, struct matrix4* world_to_camera, double screen_width, double screen_height, int raster_width, int raster_height, struct vector3i* output, int num_vertices) {
	int i;
	for(i = 0; i < num_vertices; i++) {
		world_to_raster(&world[i], world_to_camera, screen_width, screen_height, raster_width, raster_height, &output[i]);
	}
}

void draw_triangle(SDL_Renderer* renderer, struct vector3i* v1, struct vector3i* v2, struct vector3i* v3, int window_width, int window_height, double* zbuffer) {
	
	//find the bounding box of the triangle
	int min_x = window_width;
	int max_x = 0;
	int min_y = window_height;
	int max_y = 0;
	if(v1->x < min_x) min_x = v1->x;
	if(v2->x < min_x) min_x = v2->x;
	if(v3->x < min_x) min_x = v3->x;
	if(v1->x > max_x) max_x = v1->x;
	if(v2->x > max_x) max_x = v2->x;
	if(v3->x > max_x) max_x = v3->x;
	if(v1->y < min_y) min_y = v1->y;
	if(v2->y < min_y) min_y = v2->y;
	if(v3->y < min_y) min_y = v3->y;
	if(v1->y > max_y) max_y = v1->y;
	if(v2->y > max_y) max_y = v2->y;
	if(v3->y > max_y) max_y = v3->y;
	if(min_x < 0) min_x = 0;
	if(max_x > window_width) max_x = window_width;
	if(min_y < 0) min_y = 0;
	if(max_y > window_height) max_y = window_height;
		
	int i, j;
	for(j = min_y; j < max_y; j++) {
		for(i = min_x; i < max_x; i++) {
			int drawn = 0;
			
			double barycentric[3];
			barycentric[0] = (double)((v2->y - v3->y) * (i - v3->x) + (v3->x - v2->x) * (j - v3->y)) /
			                 (double)((v2->y - v3->y) * (v1->x - v3->x) + (v3->x - v2->x) * (v1->y - v3->y));
			
			barycentric[1] = (double)((v3->y - v1->y) * (i - v3->x) + (v1->x - v3->x) * (j - v3->y)) /
			                 (double)((v2->y - v3->y) * (v1->x - v3->x) + (v3->x - v2->x) * (v1->y - v3->y));
			
			barycentric[2] = 0.1 - barycentric[0] - barycentric[1];
			
			
			if(barycentric[0] > 0 && barycentric[0] < 1 && barycentric[1] > 0 && barycentric[1] < 1 && barycentric[2] > 0 && barycentric[2] < 1) {				
				//z-coordinate and z-buffer checking
				double z_coord = barycentric[0] * (v1->z) + barycentric[1] * (v2->z) + barycentric[2] * (v3->z);
				if(z_coord <= 0 || z_coord > zbuffer[i + j * window_width]) {
					continue;
				} else {
					zbuffer[i + j * window_width] = z_coord;
				}
				
				/* double z_fac = 0.1;
				if(z_coord > 1000.0) {
					z_fac = 0;
				} else if(z_coord < 0) {
					z_fac = 0.1;
				} else {
					z_fac = 0.1 - (z_coord / 1000.0);
				}
				Uint8 col = z_fac * 255; */
				if(SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255) < 0) {
					printf("Failed to set renderer color: %s\n", SDL_GetError());
					return;
				}
				
				SDL_RenderDrawPoint(renderer, i, j);
				
				drawn = 1;
			} else if(drawn == 1) {
				//skip the rest of this row
				break;
			}
		}
	}
}

void draw_triangles(SDL_Renderer* renderer, struct obj_3di* triangles, int window_width, int window_height, double* zbuffer) {
	struct vector3i* vertices = triangles->vertices;
	int* triangle_vertices_indices = triangles->triangle_vertices_indices;
	int num_triangles = triangles->num_triangles;
		
	int i;
	for(i = 0; i < num_triangles; i++) {
		struct vector3i* v1;
		struct vector3i* v2;
		struct vector3i* v3;
		v1 = &vertices[triangle_vertices_indices[i*3]];
		v2 = &vertices[triangle_vertices_indices[i*3+1]];
		v3 = &vertices[triangle_vertices_indices[i*3+2]];
		
		draw_triangle(renderer, v1, v2, v3, window_width, window_height, zbuffer);
	}
}

void draw_lines(SDL_Renderer* renderer, struct obj_3di* triangles) {
	struct vector3i* vertices = triangles->vertices;
	int* triangle_vertices_indices = triangles->triangle_vertices_indices;
	int num_triangles = triangles->num_triangles;
	
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

	int i;
	for(i = 0; i < num_triangles; i++) {
		struct vector3i* v1;
		struct vector3i* v2;
		struct vector3i* v3;
		v1 = &vertices[triangle_vertices_indices[i*3]];
		v2 = &vertices[triangle_vertices_indices[i*3+1]];
		v3 = &vertices[triangle_vertices_indices[i*3+2]];
		
		SDL_RenderDrawLine(renderer, v1->x, v1->y, v2->x, v2->y);
		SDL_RenderDrawLine(renderer, v1->x, v1->y, v3->x, v3->y);
		SDL_RenderDrawLine(renderer, v2->x, v2->y, v3->x, v3->y);;
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
	SDL_Renderer* renderer = args->renderer;
	int window_width = args->window_width;
	int window_height = args->window_height;
	//struct bouncing_point_state* bpstate = args->bpstate;
	struct obj_3di* triangles = args->triangles;
	double* zbuffer = args->zbuffer;
	
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
		
		//clear z-buffer
		int i = 0;
		for(i = 0; i < window_width * window_height; i++) {
			zbuffer[i] = INFINITY;
		}
		
		//draw triangles
		draw_triangles(renderer, triangles, window_width, window_height, zbuffer);
		
		//draw lines
		draw_lines(renderer, triangles);
		
		//update point
		//update_bouncing_point(bpstate);
		
		//draw point
		//draw_bouncing_point(renderer, bpstate);
		
		//update screen
		SDL_RenderPresent(renderer);
		
		(*framecount)++;
		
		SDL_Delay(*delay);
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

	double* zbuffer = malloc(width * height * sizeof(double));
	if(zbuffer == NULL) {
		printf("Failed to allocate memory for z-buffer\n");
		return 1;
	} else {
		int i = 0;
		for(i = 0; i < width * height; i++) {
			zbuffer[i] = INFINITY;
		}
	}
		
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
	
	//a triangle in world space
	int num_vertices = 3;
	int num_triangles = 1;
	struct vector3 vertices_orig[] = {{-100, 35, -6.1}, {0.0, 70, -6.1}, {100, 35, -6.1}};
	int tvindices[] = {0, 1, 2};
	
	//a cube in world space
	/* int num_vertices = 8;
	int num_triangles = 12;
	struct vector4 vertices_orig[] = {{-0.500000, -0.500000, 0.500000, 1}, {0.500000, -0.500000, 0.500000, 1}, {-0.500000, 0.500000, 0.500000, 1},
	                                         {0.500000, 0.500000, 0.500000, 1}, {-0.500000, 0.500000, -0.500000, 1}, {0.500000, 0.500000, -0.500000, 1},
	                                         {-0.500000, -0.500000, -0.500000, 1}, {0.500000, -0.500000, -0.500000, 1}};
	int tvindices[] = {0, 1, 2, 2, 1, 3, 2, 3, 4, 4, 3, 5, 4, 5, 6, 6, 5, 7, 6, 7, 0, 0, 7, 1, 1, 7, 3, 3, 7, 5, 6, 0, 4, 4, 0, 2}; */
	
	//camera parameters
	struct vector3 camera_from = {0.0, 0.0, 0};
	struct vector3 camera_to = {0.0, 0.0, -1.0};
	struct vector3 camera_up = {0, 1.0, 0};
	struct matrix4 camera_to_world;
	struct matrix4 world_to_camera;
	lookat(&camera_from, &camera_to, &camera_up, &camera_to_world);
	matrix4_invert(&camera_to_world, &world_to_camera);
	
	//cube converted to raster space
	struct vector3i vertices_2d[num_vertices];
	obj_to_raster((struct vector3*)vertices_orig, &world_to_camera, 128, 72, width, height, vertices_2d, num_vertices);
	
	struct obj_3di triangles_2d;
	triangles_2d.vertices = vertices_2d;
	triangles_2d.num_vertices = num_vertices;
	triangles_2d.triangle_vertices_indices = tvindices;
	triangles_2d.num_triangles = num_triangles;
	
	struct draw_thread_args dtargs = {1, &delay, &i, renderer, width, height, &bpstate, &triangles_2d, zbuffer};
	
	SDL_Thread* dtthread = SDL_CreateThread(draw_thread, "draw_thread", &dtargs);
	if(dtthread == NULL) {
		printf("Failed to create draw thread: %s\n", SDL_GetError());
		return 1;
	}
	
	/* double sin2 = 0.0348994967;
	double cos2 = 0.99939082701;
	double sinm2 = -0.0348994967;
	double cosm2 = 0.99939082701; */
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
					delay += 1;
				} else if(ev->keysym.sym == SDLK_DOWN) {
					delay -= 1;
				} else if(ev->keysym.sym == SDLK_w) {
					camera_from.z -= 0.1;
				} else if(ev->keysym.sym == SDLK_s) {
					camera_from.z += 0.1;
				} else if(ev->keysym.sym == SDLK_a) {
					camera_from.x -= 10.0;
				} else if(ev->keysym.sym == SDLK_d) {
					camera_from.x += 10.0;
				} else if(ev->keysym.sym == SDLK_q) {
					camera_from.y -= 0.1;
				} else if(ev->keysym.sym == SDLK_e) {
					camera_from.y += 0.1;
				} else if(ev->keysym.sym == SDLK_i) {
					camera_to.z -= 0.1;
				} else if(ev->keysym.sym == SDLK_k) {
					camera_to.z += 0.1;
				} else if(ev->keysym.sym == SDLK_j) {
					camera_to.x -= 10.0;
				} else if(ev->keysym.sym == SDLK_l) {
					camera_to.x += 10.0;
				} else if(ev->keysym.sym == SDLK_u) {
					camera_to.y -= 0.1;
				} else if(ev->keysym.sym == SDLK_o) {
					camera_to.y += 0.1;
				} else if(ev->keysym.sym == SDLK_r) {
					camera_from = (struct vector3){0.0, 0.0, 0.0};
					camera_to = (struct vector3){0.0, 0.0, -0.1};
				}
				
				lookat(&camera_from, &camera_to, &camera_up, &camera_to_world);
				matrix4_invert(&camera_to_world, &world_to_camera);
				obj_to_raster((struct vector3*)vertices_orig, &world_to_camera, 128, 72, width, height, vertices_2d, num_vertices);
				
				putchar('\n');
				print_vector3(&camera_from);
				print_vector3(&camera_to);
				print_vector3(&camera_up);
				putchar('\n');
				print_matrix4(&camera_to_world);
				putchar('\n');
				print_matrix4(&world_to_camera);
				putchar('\n');
				int cnt;
				for(cnt = 0; cnt < num_vertices; cnt++) {
					print_vector3i(&vertices_2d[cnt]);
				}
				putchar('\n');
			}
			
			printf("\r                                        \rEvent type: %d, delay: %u ms", event.type, delay);
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
