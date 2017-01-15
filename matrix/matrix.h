#ifndef MATRIX_INCLUDED
#define MATRIX_INCLUDED

#include <math.h>

#ifndef M_PI
#define M_PI 3.1415926535
#endif

struct vector3 {
	double x;
	double y;
	double z;
};

struct vector3i {
	int x;
	int y;
	double z;
};

struct vector4 {
	double x;
	double y;
	double z;
	double w;
};

struct matrix3 {
	double value[3][3];
};

struct matrix4 {
	double value[4][4];
};

double degree_to_radian(double degree);

double radian_to_degree(double radian);

void matrix3_mul(struct matrix3* left, struct matrix3* right, struct matrix3* output);

void matrix4_mul(struct matrix4* left, struct matrix4* right, struct matrix4* output);

void vector3_add(struct vector3* left, struct vector3* right, struct vector3* output);

void vector3_sub(struct vector3* left, struct vector3* right, struct vector3* output);

void vector3_normalize(struct vector3* input, struct vector3* output);

double vector3_dot(struct vector3* left, struct vector3* right);

void vector3_cross(struct vector3* left, struct vector3* right, struct vector3* output);

void vector3_mul_matrix4(struct vector3* vec, struct matrix4* mat, struct vector3* output);

void vector4_mul_matrix4(struct vector4* vec, struct matrix4* mat, struct vector4* output);

int matrix4_invert(struct matrix4* input, struct matrix4* output);

//lookat function to produce a world to camera matrix
void lookat(struct vector3* from, struct vector3* to, struct vector3* up, struct matrix4* output);

//fpscam function to produce a camera to world matrix
void fpscam(struct vector3* from, double pitch, double yaw, struct matrix4* output);

void camera_to_screen(struct vector3* input, struct vector3* output);

void screen_to_ndc(struct vector3* input, double width, double height, struct vector3* output);

void ndc_to_raster(struct vector3* input, int width, int height, struct vector3i* output);

void world_to_raster(struct vector3* world, struct matrix4* world_to_camera, double screen_width, double screen_height, int raster_width, int raster_height, struct vector3i* output);

#endif
