#include "matrix.h"

double degree_to_radian(double degree) {
	return M_PI * (degree / 180.0);
}

double radian_to_degree(double radian) {
	return radian * (180.0 / M_PI);
}

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
	double oon = 1.0 / norm;
	output->x = (input->x) * oon;
	output->y = (input->y) * oon;
	output->z = (input->z) * oon;
}

double vector3_dot(struct vector3* left, struct vector3* right) {
	return (left->x) * (right->x) + (left->y) * (right->y) + (left->z) * (right->z);
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

static int gluInvertMatrix(const double m[16], double invOut[16]) {
    double inv[16], det;
    int i;

    inv[0] = m[5]  * m[10] * m[15] - 
             m[5]  * m[11] * m[14] - 
             m[9]  * m[6]  * m[15] + 
             m[9]  * m[7]  * m[14] +
             m[13] * m[6]  * m[11] - 
             m[13] * m[7]  * m[10];

    inv[4] = -m[4]  * m[10] * m[15] + 
              m[4]  * m[11] * m[14] + 
              m[8]  * m[6]  * m[15] - 
              m[8]  * m[7]  * m[14] - 
              m[12] * m[6]  * m[11] + 
              m[12] * m[7]  * m[10];

    inv[8] = m[4]  * m[9] * m[15] - 
             m[4]  * m[11] * m[13] - 
             m[8]  * m[5] * m[15] + 
             m[8]  * m[7] * m[13] + 
             m[12] * m[5] * m[11] - 
             m[12] * m[7] * m[9];

    inv[12] = -m[4]  * m[9] * m[14] + 
               m[4]  * m[10] * m[13] +
               m[8]  * m[5] * m[14] - 
               m[8]  * m[6] * m[13] - 
               m[12] * m[5] * m[10] + 
               m[12] * m[6] * m[9];

    inv[1] = -m[1]  * m[10] * m[15] + 
              m[1]  * m[11] * m[14] + 
              m[9]  * m[2] * m[15] - 
              m[9]  * m[3] * m[14] - 
              m[13] * m[2] * m[11] + 
              m[13] * m[3] * m[10];

    inv[5] = m[0]  * m[10] * m[15] - 
             m[0]  * m[11] * m[14] - 
             m[8]  * m[2] * m[15] + 
             m[8]  * m[3] * m[14] + 
             m[12] * m[2] * m[11] - 
             m[12] * m[3] * m[10];

    inv[9] = -m[0]  * m[9] * m[15] + 
              m[0]  * m[11] * m[13] + 
              m[8]  * m[1] * m[15] - 
              m[8]  * m[3] * m[13] - 
              m[12] * m[1] * m[11] + 
              m[12] * m[3] * m[9];

    inv[13] = m[0]  * m[9] * m[14] - 
              m[0]  * m[10] * m[13] - 
              m[8]  * m[1] * m[14] + 
              m[8]  * m[2] * m[13] + 
              m[12] * m[1] * m[10] - 
              m[12] * m[2] * m[9];

    inv[2] = m[1]  * m[6] * m[15] - 
             m[1]  * m[7] * m[14] - 
             m[5]  * m[2] * m[15] + 
             m[5]  * m[3] * m[14] + 
             m[13] * m[2] * m[7] - 
             m[13] * m[3] * m[6];

    inv[6] = -m[0]  * m[6] * m[15] + 
              m[0]  * m[7] * m[14] + 
              m[4]  * m[2] * m[15] - 
              m[4]  * m[3] * m[14] - 
              m[12] * m[2] * m[7] + 
              m[12] * m[3] * m[6];

    inv[10] = m[0]  * m[5] * m[15] - 
              m[0]  * m[7] * m[13] - 
              m[4]  * m[1] * m[15] + 
              m[4]  * m[3] * m[13] + 
              m[12] * m[1] * m[7] - 
              m[12] * m[3] * m[5];

    inv[14] = -m[0]  * m[5] * m[14] + 
               m[0]  * m[6] * m[13] + 
               m[4]  * m[1] * m[14] - 
               m[4]  * m[2] * m[13] - 
               m[12] * m[1] * m[6] + 
               m[12] * m[2] * m[5];

    inv[3] = -m[1] * m[6] * m[11] + 
              m[1] * m[7] * m[10] + 
              m[5] * m[2] * m[11] - 
              m[5] * m[3] * m[10] - 
              m[9] * m[2] * m[7] + 
              m[9] * m[3] * m[6];

    inv[7] = m[0] * m[6] * m[11] - 
             m[0] * m[7] * m[10] - 
             m[4] * m[2] * m[11] + 
             m[4] * m[3] * m[10] + 
             m[8] * m[2] * m[7] - 
             m[8] * m[3] * m[6];

    inv[11] = -m[0] * m[5] * m[11] + 
               m[0] * m[7] * m[9] + 
               m[4] * m[1] * m[11] - 
               m[4] * m[3] * m[9] - 
               m[8] * m[1] * m[7] + 
               m[8] * m[3] * m[5];

    inv[15] = m[0] * m[5] * m[10] - 
              m[0] * m[6] * m[9] - 
              m[4] * m[1] * m[10] + 
              m[4] * m[2] * m[9] + 
              m[8] * m[1] * m[6] - 
              m[8] * m[2] * m[5];

    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    if (det == 0)
        return -1;

    det = 1.0 / det;

    for (i = 0; i < 16; i++)
        invOut[i] = inv[i] * det;

    return 0;
}

int matrix4_invert(struct matrix4* input, struct matrix4* output) {
	return gluInvertMatrix(&(input->value[0][0]), &(output->value[0][0]));
}

//lookat function to produce a camera to world matrix
void lookat(struct vector3* from, struct vector3* to, struct vector3* up, struct matrix4* output) {
	struct vector3 forward, right;
	struct vector3 forward_normalized, right_normalized, up_normalized;
	
	//forward
	vector3_sub(from, to, &forward);
	vector3_normalize(&forward, &forward_normalized);
	
	//right
	vector3_cross(up, &forward, &right);
	vector3_normalize(&right, &right_normalized);
	
	//up
	vector3_cross(&forward_normalized, &right_normalized, &up_normalized);
	
	output->value[0][0] = right_normalized.x;
	output->value[0][1] = right_normalized.y;
	output->value[0][2] = right_normalized.z;
	output->value[0][3] = 0;
	
	output->value[1][0] = up_normalized.x;
	output->value[1][1] = up_normalized.y;
	output->value[1][2] = up_normalized.z;
	output->value[1][3] = 0;
	
	output->value[2][0] = forward_normalized.x;
	output->value[2][1] = forward_normalized.y;
	output->value[2][2] = forward_normalized.z;
	output->value[2][3] = 0;
	
	output->value[3][0] = from->x;
	output->value[3][1] = from->y;
	output->value[3][2] = from->z;
	output->value[3][3] = 1.0;
}

//fpscam function to produce a camera to world matrix
void fpscam(struct vector3* from, double pitch, double yaw, struct matrix4* output) {
	//pitch shoule be between -pi/2 and pi/2
	//yaw should be between 0 and 2*pi
	
	double pitch_sin = sin(pitch);
	double pitch_cos = cos(pitch);
	double yaw_sin = sin(yaw);
	double yaw_cos = cos(yaw);
	
	output->value[0][0] = yaw_cos;
	output->value[0][1] = 0;
	output->value[0][2] = -yaw_sin;
	output->value[0][3] = 0;
	
	output->value[1][0] = yaw_sin * pitch_sin;
	output->value[1][1] = pitch_cos;
	output->value[1][2] = yaw_cos * pitch_sin;
	output->value[1][3] = 0;
	
	output->value[2][0] = yaw_sin * pitch_cos;
	output->value[2][1] = -pitch_sin;
	output->value[2][2] = yaw_cos * pitch_cos;
	output->value[2][3] = 0;
	
	output->value[3][0] = from->x;
	output->value[3][1] = from->y;
	output->value[3][2] = from->z;
	output->value[3][3] = 1.0;
}

//convert camera space to screen space
int camera_to_screen(struct vector3* input, double width, double height, struct vector3* output) {
	//perspective divide
	output->x = (input->x) / -(input->z);
	output->y = (input->y) / -(input->z);
	output->z = -(input->z);
	
	if(fabs(output->x) > width || fabs(output->y) > height) {
		return -1;
	}
	
	return 0;
}

//convert screen space to NDC space
void screen_to_ndc(struct vector3* input, double width, double height, struct vector3* output) {
	output->x = ((input->x) + (width / 2.0)) / width;
	output->y = ((input->y) + (height / 2.0)) / height;
	output->z = ((input->z) - 1.0) / 1.0;
	
	/* if(output->z > 1.0) {
		output->z = -(200.0 + output->z);
	}
	
	if(fabs(input->x) > width / 2.0 || fabs(input->y) > height / 2.0) {
		output->z = -100.0;
	} */
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
	camera_to_screen(&camera, screen_width, screen_height, &screen);
	//screen to NDC
	screen_to_ndc(&screen, screen_width, screen_height, &ndc);
	//NDC to raster
	ndc_to_raster(&ndc, raster_width, raster_height, output);
}