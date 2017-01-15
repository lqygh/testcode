#include <stdio.h>
#include "matrix.h"

int main() {
	printf("pi: %f\n\n", M_PI);
	
	//from
	struct vector3 v0 = {-10, 10, 0};
	struct vector3 v0n;
	vector3_normalize(&v0, &v0n);
	printf("from:\n");
	printf("%f %f %f\n", v0.x, v0.y, v0.z);
	printf("normalized\n");
	printf("%f %f %f\n", v0n.x, v0n.y, v0n.z);
	putchar('\n');
	
	//to
	struct vector3 v1 = {-10, 10, -1.0};
	struct vector3 v1n;
	vector3_normalize(&v1, &v1n);
	printf("to:\n");
	printf("%f %f %f\n", v1.x, v1.y, v1.z);
	printf("normalized\n");
	printf("%f %f %f\n", v1n.x, v1n.y, v1n.z);
	putchar('\n');
	
	//up
	struct vector3 up = {0, 1.0, 0};
	struct matrix4 w0, w1;
	lookat(&v0, &v1, &up, &w0);
	
	//camera to world
	printf("camera to world:\n");
	unsigned char i, j;
	for(i = 0; i < 4; i++) {
		for(j = 0; j < 4; j++) {
			printf("%f ", w0.value[i][j]);
		}
		putchar('\n');
	}
	putchar('\n');
	
	//world to camera
	printf("world to camera:\n");
	matrix4_invert(&w0, &w1);
	for(i = 0; i < 4; i++) {
		for(j = 0; j < 4; j++) {
			printf("%f ", w1.value[i][j]);
		}
		putchar('\n');
	}
	putchar('\n');
	
	struct vector3 p0 = {100, 35, -6.1};
	struct vector3i p1;
	world_to_raster(&p0, &w1, 128, 72, 1280, 720, &p1);
	printf("%f %f %f\n", p0.x, p0.y, p0.z);
	printf("%d %d %f\n", p1.x, p1.y, p1.z);
	putchar('\n');
	
	struct vector3 p2 = {-100, 35, -6.1};
	struct vector3i p3;
	world_to_raster(&p2, &w1, 128, 72, 1280, 720, &p3);
	printf("%f %f %f\n", p2.x, p2.y, p2.z);
	printf("%d %d %f\n", p3.x, p3.y, p3.z);
	putchar('\n');
	
	printf("%lu %lu %lu %lu\n", sizeof(struct matrix3), sizeof(struct matrix4), sizeof(struct vector3), sizeof(struct vector4));
	
	return 0;
}