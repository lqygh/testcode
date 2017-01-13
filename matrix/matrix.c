#include "matrix.h"

void matrix3_mul(struct matrix3* left, struct matrix3* right, struct matrix3* output) {	
	unsigned char i, j;
	for(i = 0; i < 3; i++) {
		for(j = 0; j < 3; j++) {
			output->value[i][j] = left->value[i][0] * right->value[0][j] + left->value[i][1] * right->value[1][j] + left->value[i][2] * right->value[2][j];
		}
	}
}

void matrix4_mul(struct matrix4* left, struct matrix4* right, struct matrix4* output) {
	unsigned char i, j;
	for(i = 0; i < 4; i++) {
		for(j = 0; j < 4; j++) {
			output->value[i][j] = left->value[i][0] * right->value[0][j] + left->value[i][1] * right->value[1][j] + left->value[i][2] * right->value[2][j]
			                    + left->value[i][3] * right->value[3][j];
		}
	}
}

void vector3_add(struct vector3* left, struct vector3* right, struct vector3* output) {
	output->x = (left->x) + (right->x);
	output->y = (left->y) + (right->y);
	output->z = (left->z) + (right->z);
}

void vector3_sub(struct vector3* left, struct vector3* right, struct vector3* output) {
	output->x = (left->x) - (right->x);
	output->y = (left->y) - (right->y);
	output->z = (left->z) - (right->z);
}

void vector3_normalize(struct vector3* input, struct vector3* output) {
	double norm = sqrt((input->x) * (input->x) + (input->y) * (input->y) + (input->z) * (input->z));
	double oon = 1.0/norm;
	output->x = (input->x) * oon;
	output->y = (input->y) * oon;
	output->z = (input->z) * oon;
}

void vector3_dot(struct vector3* left, struct vector3* right, double* output) {
	*output = (left->x) * (right->x) + (left->y) * (right->y) + (left->z) * (right->z);
}

void vector3_cross(struct vector3* left, struct vector3* right, struct vector3* output) {
	output->x = (left->y) * (right->z) - (left->z) * (right->y);
	output->y = (left->z) * (right->x) - (left->x) * (right->z);
	output->z = (left->x) * (right->y) - (left->y) * (right->x);
}

void vector3_mul_matrix4(struct vector3* vec, struct matrix4* mat, struct vector3* output) {
	output->x = (vec->x) * (mat->value[0][0]) + (vec->y) * (mat->value[1][0]) + (vec->z) * (mat->value[2][0]) + (mat->value[3][0]);
	output->y = (vec->x) * (mat->value[0][1]) + (vec->y) * (mat->value[1][1]) + (vec->z) * (mat->value[2][1]) + (mat->value[3][1]);
	output->z = (vec->x) * (mat->value[0][2]) + (vec->y) * (mat->value[1][2]) + (vec->z) * (mat->value[2][2]) + (mat->value[3][2]);
}

void vector4_mul_matrix4(struct vector4* vec, struct matrix4* mat, struct vector4* output) {
	output->x = (vec->x) * (mat->value[0][0]) + (vec->y) * (mat->value[1][0]) + (vec->z) * (mat->value[2][0]) + (vec->w) * (mat->value[3][0]);
	output->y = (vec->x) * (mat->value[0][1]) + (vec->y) * (mat->value[1][1]) + (vec->z) * (mat->value[2][1]) + (vec->w) * (mat->value[3][1]);
	output->z = (vec->x) * (mat->value[0][2]) + (vec->y) * (mat->value[1][2]) + (vec->z) * (mat->value[2][2]) + (vec->w) * (mat->value[3][2]);
	output->w = (vec->x) * (mat->value[0][3]) + (vec->y) * (mat->value[1][3]) + (vec->z) * (mat->value[2][3]) + (vec->w) * (mat->value[3][3]);
}

//lookat function to produce a world to camera matrix
void lookat(struct vector3* from, struct vector3* to, struct vector3* up, struct matrix4* output) {
	struct vector3 forward, right;
	struct vector3 forward_normalized, right_normalized, up_normalized;
	double fdfn, fdrn, fdun;
	
	//forward
	vector3_sub(from, to, &forward);
	vector3_normalize(&forward, &forward_normalized);
	vector3_dot(from, &forward_normalized, &fdfn);
	
	//right
	vector3_cross(up, &forward, &right);
	vector3_normalize(&right, &right_normalized);
	vector3_dot(from, &right_normalized, &fdrn);
	
	//up
	vector3_cross(&forward_normalized, &right_normalized, &up_normalized);
	vector3_dot(from, &up_normalized, &fdun);
	
	//view matrix
	output->value[0][0] = right_normalized.x;
	output->value[1][0] = right_normalized.y;
	output->value[2][0] = right_normalized.z;
	output->value[3][0] = -fdrn;
	
	output->value[0][1] = up_normalized.x;
	output->value[1][1] = up_normalized.y;
	output->value[2][1] = up_normalized.z;
	output->value[3][1] = -fdun;
	
	output->value[0][2] = forward_normalized.x;
	output->value[1][2] = forward_normalized.y;
	output->value[2][2] = forward_normalized.z;
	output->value[3][2] = -fdfn;
	
	output->value[0][3] = 0;
	output->value[1][3] = 0;
	output->value[2][3] = 0;
	output->value[3][3] = 1.0;
}

//convert camera space to screen space
void camera_to_screen(struct vector3* input, struct vector3* output) {
	//perspective divide
	output->x = (input->x) / -(input->z);
	output->y = (input->y) / -(input->z);
	output->z = -(input->z);
}

//convert screen space to NDC space
void screen_to_ndc(struct vector3* input, double width, double height, struct vector3* output) {
	output->x = ((input->x) + (width / 2)) / width;
	output->y = ((input->y) + (height / 2)) / height;
	output->z = input->z;
}

//convert NDC space to raster space
void ndc_to_raster(struct vector3* input, int width, int height, struct vector3i* output) {
	output->x = floor((input->x) * width);
	output->y = floor((1.0 - (input->y)) * height);
	output->z = input->z;
}

//convert world space to raster space
void world_to_raster(struct vector3* world, struct matrix4* world_to_camera, double screen_width, double screen_height, int raster_width, int raster_height, struct vector3i* output) {
	struct vector3 camera, screen, ndc;
	//world to camera
	vector3_mul_matrix4(world, world_to_camera, &camera);
	//camera to screen
	camera_to_screen(&camera, &screen);
	//screen to NDC
	screen_to_ndc(&screen, screen_width, screen_height, &ndc);
	//NDC to raster
	ndc_to_raster(&ndc, raster_width, raster_height, output);
}