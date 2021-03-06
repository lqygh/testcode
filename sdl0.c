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
	for(unsigned char i = 0; i < 4; i++) {
		for(unsigned char j = 0; j < 4; j++) {
			printf("%f ", mat->value[i][j]);
		}
		putchar('\n');
	}
}

void obj_to_raster(struct vector3* world, struct matrix4* world_to_camera, double screen_width, double screen_height, int raster_width, int raster_height, struct vector3i* output, int num_vertices) {
	for(int i = 0; i < num_vertices; i++) {
		world_to_raster(&world[i], world_to_camera, screen_width, screen_height, raster_width, raster_height, &output[i]);
	}
}

void camera_move(struct vector3* camera_position, struct matrix4* camera_to_world, double right, double upward, double forward) {
	camera_position->x = camera_position->x + right * camera_to_world->value[0][0] + upward * camera_to_world->value[1][0] + forward * camera_to_world->value[2][0];
	camera_position->y = camera_position->y + right * camera_to_world->value[0][1] + upward * camera_to_world->value[1][1] + forward * camera_to_world->value[2][1];
	camera_position->z = camera_position->z + right * camera_to_world->value[0][2] + upward * camera_to_world->value[1][2] + forward * camera_to_world->value[2][2];
}

void draw_triangle(SDL_Renderer* renderer, struct vector3i* v1, struct vector3i* v2, struct vector3i* v3, int window_width, int window_height, double* zbuffer) {
	//SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
	
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
			
			barycentric[2] = 1.0 - barycentric[0] - barycentric[1];
			
			
			if(barycentric[0] >= 0 && barycentric[0] <= 1 && barycentric[1] >= 0 && barycentric[1] <= 1 && barycentric[2] >= 0 && barycentric[2] <= 1) {				
				//z-coordinate and z-buffer checking
				double z_coord = barycentric[0] * (v1->z) + barycentric[1] * (v2->z) + barycentric[2] * (v3->z);
				if(z_coord < 0 || z_coord > zbuffer[i + j * window_width]) {
					continue;
				}
				
				zbuffer[i + j * window_width] = z_coord;
				
				/* double z_fac = 0.1;
				if(z_coord > 1000.0) {
					z_fac = 0;
				} else if(z_coord < 0) {
					z_fac = 0.1;
				} else {
					z_fac = 0.1 - (z_coord / 1000.0);
				}
				Uint8 col = z_fac * 255; */
				/* if(SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255) < 0) {
					printf("Failed to set renderer color: %s\n", SDL_GetError());
					return;
				} */
				
				double z_fac = 1.0 - z_coord;
				if(z_fac > 1.0) {
					z_fac = 1.0;
				} else if(z_fac < 0) {
					z_fac = 0.05;
				}
				Uint8 col = z_fac * 255;
				SDL_SetRenderDrawColor(renderer, col, col, col, 255);
				
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
		
	for(int i = 0; i < num_triangles; i++) {
		struct vector3i* v1;
		struct vector3i* v2;
		struct vector3i* v3;
		v1 = &vertices[triangle_vertices_indices[i*3]];
		v2 = &vertices[triangle_vertices_indices[i*3+1]];
		v3 = &vertices[triangle_vertices_indices[i*3+2]];
		
		if(v1->z < 0 || v2->z < 0 || v3->z < 0) {
			//skip triangles that are out of sight
			continue;
		}
		
		draw_triangle(renderer, v1, v2, v3, window_width, window_height, zbuffer);
	}
}

void draw_lines(SDL_Renderer* renderer, struct obj_3di* triangles) {
	struct vector3i* vertices = triangles->vertices;
	int* triangle_vertices_indices = triangles->triangle_vertices_indices;
	int num_triangles = triangles->num_triangles;
	
	SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);

	for(int i = 0; i < num_triangles; i++) {
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
		for(int i = 0; i < window_width * window_height; i++) {
			zbuffer[i] = INFINITY;
		}
		
		/* struct vector3i v1 = {0, 0, 5};
		struct vector3i v2 = {600, 400, 5};
		struct vector3i v3 = {0, 400, 5};
		draw_triangle(renderer, &v1, &v2, &v3, window_width, window_height, zbuffer); */
		
		//draw triangles
		draw_triangles(renderer, triangles, window_width, window_height, zbuffer);
		
		//draw lines
		//draw_lines(renderer, triangles);
		
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
	int width = 555;
	int height = 555;
	
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
		for(int i = 0; i < width * height; i++) {
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
	
	//triangles in world space
	/* int num_vertices = 12;
	int num_triangles = 6;
	struct vector3 vertices_orig[] = {{-5.0, 5.0, -2}, {-5.0, -5.0, -2}, {5.0, 5.0, -2}, {5.0, -5.0, -2},
	                                  {-5.0, 5.0, 2.5}, {-5.0, -5.0, 2.5}, {5.0, 5.0, 2.5}, {5.0, -5.0, 2.5},
									  {-5.0, 0, 2}, {-5.0, 0, -2}, {5.0, 0, 2}, {5.0, 0, -2}};
	int tvindices[] = {0, 1, 2, 1, 2, 3, 4, 5, 6, 5, 6, 7, 8, 9, 10, 9, 10, 11}; */
	
	//a cube in world space
	int num_vertices = 8;
	int num_triangles = 12;
	struct vector3 vertices_orig[] = {{-0.500000, -0.500000, 0.500000}, {0.500000, -0.500000, 0.500000}, {-0.500000, 0.500000, 0.500000},
	                                         {0.500000, 0.500000, 0.500000}, {-0.500000, 0.500000, -0.500000}, {0.500000, 0.500000, -0.500000},
	                                         {-0.500000, -0.500000, -0.500000}, {0.500000, -0.500000, -0.500000}};
	int tvindices[] = {0, 1, 2, 2, 1, 3, 2, 3, 4, 4, 3, 5, 4, 5, 6, 6, 5, 7, 6, 7, 0, 0, 7, 1, 1, 7, 3, 3, 7, 5, 6, 0, 4, 4, 0, 2};
	
	/* struct vector4 vertices_orig[] = {{-0.500000, -0.500000, 0.500000, 1}, {0.500000, -0.500000, 0.500000, 1}, {-0.500000, 0.500000, 0.500000, 1},
	                                         {0.500000, 0.500000, 0.500000, 1}, {-0.500000, 0.500000, -0.500000, 1}, {0.500000, 0.500000, -0.500000, 1},
	                                         {-0.500000, -0.500000, -0.500000, 1}, {0.500000, -0.500000, -0.500000, 1}}; */
	
	//camera parameters
	struct vector3 camera_from = {0.0, 0.0, 0.0};
	//struct vector3 camera_to = {0.0, 0.0, -1.0};
	//struct vector3 camera_up = {0, 1.0, 0};
	double pitch_deg = 0.0;
	double yaw_deg = 0.0;
	struct matrix4 camera_to_world;
	struct matrix4 world_to_camera;
	fpscam(&camera_from, degree_to_radian(pitch_deg), degree_to_radian(yaw_deg), &camera_to_world);
	matrix4_invert(&camera_to_world, &world_to_camera);
	
	//convert to raster space
	struct vector3i vertices_2d[num_vertices];
	double screen_width = 2.0;
	double screen_height = 2.0;
	obj_to_raster((struct vector3*)vertices_orig, &world_to_camera, screen_width, screen_height, width, height, vertices_2d, num_vertices);
	
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
	
	SDL_Event event;
	int camera_heading_x = 0;
	int camera_heading_y = 0;
	int camera_heading_z = 0;
	int camera_heading_pitch = 0;
	int camera_heading_yaw = 0;
	while(dtargs.should_run) {
		if(SDL_PollEvent(&event) == 1) {
			if(event.type == SDL_QUIT) {
				printf("\nSDL_QUIT event occurred\n");
				dtargs.should_run = 0;
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
				if(ev->keysym.sym == SDLK_n) {
					delay -= 1;
				} else if(ev->keysym.sym == SDLK_m) {
					delay += 1;
				} else if(ev->keysym.sym == SDLK_w) {
					//camera_move(&camera_from, &camera_to_world, 0, 0, -0.1);
					camera_heading_z = -1;
				} else if(ev->keysym.sym == SDLK_s) {
					//camera_move(&camera_from, &camera_to_world, 0, 0, 0.1);
					camera_heading_z = 1;
				} else if(ev->keysym.sym == SDLK_a) {
					//camera_move(&camera_from, &camera_to_world, -0.1, 0, 0);
					camera_heading_x = -1;
				} else if(ev->keysym.sym == SDLK_d) {
					//camera_move(&camera_from, &camera_to_world, 0.1, 0, 0);
					camera_heading_x = 1;
				} else if(ev->keysym.sym == SDLK_q) {
					//camera_move(&camera_from, &camera_to_world, 0, -0.1, 0);
					camera_heading_y = -1;
				} else if(ev->keysym.sym == SDLK_e) {
					//camera_move(&camera_from, &camera_to_world, 0, 0.1, 0);
					camera_heading_y = 1;
				} else if(ev->keysym.sym == SDLK_i) {
					//pitch_deg += 1;
					camera_heading_pitch = 1;
					//if(pitch_deg > 90.0) pitch_deg = 90.0;
				} else if(ev->keysym.sym == SDLK_k) {
					//pitch_deg -= 1;
					camera_heading_pitch = -1;
					//if(pitch_deg < -90.0) pitch_deg = -90.0;
				} else if(ev->keysym.sym == SDLK_j) {
					//yaw_deg += 1;
					camera_heading_yaw = 1;
					//if(yaw_deg > 360.0) yaw_deg = 0.0;
				} else if(ev->keysym.sym == SDLK_l) {
					//yaw_deg -= 1;
					camera_heading_yaw = -1;
					//if(yaw_deg < 0) yaw_deg = 360.0;
				} else if(ev->keysym.sym == SDLK_UP) {
					//screen_height += 0.1;
				} else if(ev->keysym.sym == SDLK_DOWN) {
					//screen_height -= 0.1;
					//if(screen_height < 0) screen_height = 5;
				} else if(ev->keysym.sym == SDLK_LEFT) {
					//screen_width -= 0.1;
					//if(screen_width < 0) screen_width = 5;
				} else if(ev->keysym.sym == SDLK_RIGHT) {
					//screen_width += 0.1;
				} else if(ev->keysym.sym == SDLK_t) {
					pitch_deg = 0;
					yaw_deg = 0;
				} else if(ev->keysym.sym == SDLK_r) {
					camera_from = (struct vector3){0.0, 0.0, 0.0};
					pitch_deg = 0;
					yaw_deg = 0;
					screen_width = 2.0;
					screen_height = 2.0;
				}
				
			} else if(event.type == SDL_KEYUP) {
				SDL_KeyboardEvent* ev = &(event.key);
				if(ev->keysym.sym == SDLK_w) {
					//camera_move(&camera_from, &camera_to_world, 0, 0, -0.1);
					camera_heading_z = 0;
				} else if(ev->keysym.sym == SDLK_s) {
					//camera_move(&camera_from, &camera_to_world, 0, 0, 0.1);
					camera_heading_z = 0;
				} else if(ev->keysym.sym == SDLK_a) {
					//camera_move(&camera_from, &camera_to_world, -0.1, 0, 0);
					camera_heading_x = 0;
				} else if(ev->keysym.sym == SDLK_d) {
					//camera_move(&camera_from, &camera_to_world, 0.1, 0, 0);
					camera_heading_x = 0;
				} else if(ev->keysym.sym == SDLK_q) {
					//camera_move(&camera_from, &camera_to_world, 0, -0.1, 0);
					camera_heading_y = 0;
				} else if(ev->keysym.sym == SDLK_e) {
					//camera_move(&camera_from, &camera_to_world, 0, 0.1, 0);
					camera_heading_y = 0;
				} else if(ev->keysym.sym == SDLK_i) {
					//pitch_deg += 1;
					camera_heading_pitch = 0;
					//if(pitch_deg > 90.0) pitch_deg = 90.0;
				} else if(ev->keysym.sym == SDLK_k) {
					//pitch_deg -= 1;
					camera_heading_pitch = 0;
					//if(pitch_deg < -90.0) pitch_deg = -90.0;
				} else if(ev->keysym.sym == SDLK_j) {
					//yaw_deg += 1;
					camera_heading_yaw = 0;
					//if(yaw_deg > 360.0) yaw_deg = 0.0;
				} else if(ev->keysym.sym == SDLK_l) {
					//yaw_deg -= 1;
					camera_heading_yaw = 0;
					//if(yaw_deg < 0) yaw_deg = 360.0;
				}
			}
		}
		
		camera_move(&camera_from, &camera_to_world, 0.1 * (double)camera_heading_x, 0.1 * (double)camera_heading_y, 0.1 * (double)camera_heading_z);
		pitch_deg = pitch_deg + 1.0 * (double)camera_heading_pitch;
		yaw_deg = yaw_deg + 1.0 * (double)camera_heading_yaw;
		if(pitch_deg > 90.0) pitch_deg = 90.0;
		if(pitch_deg < -90.0) pitch_deg = -90.0;
		if(yaw_deg > 360.0) yaw_deg = 0.0;
		if(yaw_deg < 0) yaw_deg = 360.0;
				
		//lookat(&camera_from, &camera_to, &camera_up, &camera_to_world);
		fpscam(&camera_from, degree_to_radian(pitch_deg), degree_to_radian(yaw_deg), &camera_to_world);
		matrix4_invert(&camera_to_world, &world_to_camera);
		obj_to_raster((struct vector3*)vertices_orig, &world_to_camera, screen_width, screen_height, width, height, vertices_2d, num_vertices);
		
		putchar('\n');
		
		printf("camera to world:\n");
		print_matrix4(&camera_to_world);
		putchar('\n');
		
		printf("world to camera:\n");
		print_matrix4(&world_to_camera);
		putchar('\n');
		
		printf("pitch: %f, yaw: %f\n", pitch_deg, yaw_deg);
		putchar('\n');
			
		//printf("screen_width: %f, screen_height: %f\n", screen_width, screen_height);
		//putchar('\n');
			
		printf("camera:\n");
		print_vector3(&camera_from);
		putchar('\n');
		
		printf("raster vertices:\n");
		for(int i = 0; i < num_vertices; i++) {
			print_vector3i(&vertices_2d[i]);
		}
		putchar('\n');

		printf("\r                                        \rEvent type: %d, delay: %u ms", event.type, delay);
		fflush(stdout);
		
		SDL_Delay(20);
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
