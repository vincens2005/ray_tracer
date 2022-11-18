#ifndef SCENES
#define SCENES
#include "world.h"
#include "raylib.h"
#include "raymath.h"
#include "utils.h"
#include "camera.h"

HittableList sexy_scene() {
	HittableList world = MakeHittableList();

	// make materials
	printf("making materials\r\n");
	int lambertiangray = HittableList_addMat(&world, MakeLambertian(color(0.7, 0.7, 0.7)));
	int redmetal = HittableList_addMat(&world, MakeMetal(color(0.7, 0.3, 0.3), 0.05));
	int metal = HittableList_addMat(&world, MakeMetal(color(0.8, 0.8, 0.8), 0.3));
	int glass = HittableList_addMat(&world, MakeDielectric(1.51));
	int ground = HittableList_addMat(&world, MakeLambertian(color(0.2, 0.6, 0.1)));
	int purpleglow = HittableList_addMat(&world, MakeEmissive(color(0.8, 0.0, 1.0), 10));

	printf("making balls\r\n");
	HittableList_add(&world, MakeSphere(point3(0, -100.5, -1), 100, ground));

	HittableList_add(&world, MakeSphere(point3(-1, 0, -1), 0.5, glass));
	HittableList_add(&world, MakeSphere(point3(0, 0, -1), 0.5, lambertiangray));
	HittableList_add(&world, MakeSphere(point3(1, 0, -1), 0.5, redmetal));

	HittableList_add(&world, MakeSphere(point3(0.1, 1, -1.5), 0.34, metal));
	HittableList_add(&world, MakeSphere(point3(0.1, 1, 0.5), 0.34, purpleglow));

	printf("balls initialized\r\n\tworld:\r\n");
	HittableList_print(&world, "\t");


	Vector3 lookfrom = vec3(3, 3, 2);
	Vector3 lookat = vec3(0, 0, -1);

	world.camera = MakeCamera(
		lookfrom, // origin
		lookat, // look at
		vec3(0, 1, 0), // up
		50, // fov
		0, // aperture
		Vector3Length(Vector3Subtract(lookfrom, lookat)), // we focus on the point we're looking at
		0, // image width
		0 // image height
	);
	return world;
}

#endif
