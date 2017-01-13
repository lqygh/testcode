#include <stdio.h>
#include <stdint.h>
#include <math.h>

struct vector3 {
	double x;
	double y;
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

void matrix3_mul(struct matrix3* left, struct matrix3* right, struct matrix3* output) {	
	uint8_t i, j;
	for(i = 0; i < 3; i++) {
		for(j = 0; j < 3; j++) {
			//o[i*3+j] = l[i*3] * r[j] + l[i*3+1] * r[j+3] + l[i*3+2] * r[j+6];
			output->value[i][j] = left->value[i][0] * right->value[0][j] + left->value[i][1] * right->value[1][j] + left->value[i][2] * right->value[2][j];
		}
	}
}

void matrix4_mul(struct matrix4* left, struct matrix4* right, struct matrix4* output) {
	uint8_t i, j;
	for(i = 0; i < 4; i++) {
		for(j = 0; j < 4; j++) {
			//o[i*4+j] = l[i*4] * r[j] + l[i*4+1] * r[j+4] + l[i*4+2] * r[j+8] + l[i*4+3] * r[j+12];
			output->value[i][j] = left->value[i][0] * right->value[0][j] + left->value[i][1] * right->value[1][j] + left->value[i][2] * right->value[2][j]
			                    + left->value[i][3] * right->value[3][j];
		}
	}
}

/* void matrix_inv3(struct matrix3* input, struct matrix3* output) {
	
}

void matrix_inv4(struct matrix4* input, struct matrix4* output) {
	
} */

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
	double product = (left->x) * (right->x) + (left->y) * (right->y) + (left->z) * (right->z);
	*output = product;
}

void vector3_cross(struct vector3* left, struct vector3* right, struct vector3* output) {
	output->x = (left->y) * (right->z) - (left->z) * (right->y);
	output->y = (left->z) * (right->x) - (left->x) * (right->z);
	output->z = (left->x) * (right->y) - (left->y) * (right->x);
}

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

int main() {
	struct vector3 v0 = {5.1, 3.9, 2.2};
	struct vector3 v0n;
	vector3_normalize(&v0, &v0n);
	
	printf("%f %f %f\n", v0.x, v0.y, v0.z);
	printf("%f %f %f\n", v0n.x, v0n.y, v0n.z);
	
	struct vector3 v1 = {50.1, 30.9, 20.2};
	struct vector3 v1n;
	vector3_normalize(&v1, &v1n);
	
	printf("%f %f %f\n", v1.x, v1.y, v1.z);
	printf("%f %f %f\n", v1n.x, v1n.y, v1n.z);
	
	struct vector3 up = {0, 1.0, 0};
	struct matrix4 vm;
	lookat(&v0, &v1, &up, &vm);
	
	uint8_t i, j;
	for(i = 0; i < 4; i++) {
		for(j = 0; j < 4; j++) {
			printf("%f ", vm.value[i][j]);
		}
		putchar('\n');
	}
	
	printf("%lu %lu %lu %lu\n", sizeof(struct matrix3), sizeof(struct matrix4), sizeof(struct vector3), sizeof(struct vector4));
	
	return 0;
}