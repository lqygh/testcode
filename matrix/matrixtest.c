#include <stdio.h>
#include "matrix.h"

int main() {
	struct vector3 v0 = {5.1, 3.9, 2.2};
	struct vector3 v0n;
	vector3_normalize(&v0, &v0n);
	printf("%f %f %f\n", v0.x, v0.y, v0.z);
	printf("%f %f %f\n", v0n.x, v0n.y, v0n.z);
	putchar('\n');
	
	struct vector3 v1 = {50.1, 30.9, 20.2};
	struct vector3 v1n;
	vector3_normalize(&v1, &v1n);
	printf("%f %f %f\n", v1.x, v1.y, v1.z);
	printf("%f %f %f\n", v1n.x, v1n.y, v1n.z);
	putchar('\n');
	
	struct vector3 up = {0, 1.0, 0};
	struct matrix4 vm;
	lookat(&v0, &v1, &up, &vm);
	
	unsigned char i, j;
	for(i = 0; i < 4; i++) {
		for(j = 0; j < 4; j++) {
			printf("%f ", vm.value[i][j]);
		}
		putchar('\n');
	}
	putchar('\n');
	
	struct vector3 p0 = {3.3, 2.7, 9.1};
	struct vector3i p1;
	world_to_raster(&p0, &vm, 1280, 720, 1920, 1080, &p1);
	printf("%f %f %f\n", p0.x, p0.y, p0.z);
	printf("%d %d %f\n", p1.x, p1.y, p1.z);
	putchar('\n');
	
	printf("%lu %lu %lu %lu\n", sizeof(struct matrix3), sizeof(struct matrix4), sizeof(struct vector3), sizeof(struct vector4));
	
	return 0;
}