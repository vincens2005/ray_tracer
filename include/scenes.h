#ifndef SCENES
#define SCENES
#include "hittable_list.h"
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

	printf("building BVH...\r\n");

	Hittable BVH = MakeBVHNode(world.objects, 0, world.len);

	printf("BVH built!\r\n\tworld:\r\n");
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

HittableList random_scene() {
	printf("generating scene...\r\n");
	HittableList world = MakeHittableList();
	int ground_material = HittableList_addMat(&world, MakeLambertian(color(0.5, 0.5, 0.5)));
	HittableList_add(&world, MakeSphere(point3(0, 0, 0), 1000, ground_material));

	for (int a = -11; a < 11; a++) {
		for (int b = -11; b < 11; b++) {
			double choose_mat = random_double1();
			Vector3 center = vec3(a + 0.9*random_double1(), 0.2, b + 0.9*random_double1());

			if (Vector3Length(Vector3Subtract(center, point3(4, 0.2, 0))) > 0.9) {
				if (choose_mat < 0.8) {
					// lambertian
					Vector3 albedo = Vector3Multiply(random_color(), random_color());
					int m = HittableList_addMat(&world, MakeLambertian(albedo));
					HittableList_add(&world, MakeSphere(center, 0.2, m));
				}
				else if (choose_mat < 0.95) {
					// metal
					Vector3 albedo = Vector3RandRange(0.5, 1);
					float r = random_double1();
					int m = HittableList_addMat(&world, MakeMetal(albedo, r));
					HittableList_add(&world, MakeSphere(center, 0.2, m));
				}
				else {
					int m = HittableList_addMat(&world, MakeDielectric(1.5));
					HittableList_add(&world, MakeSphere(center, 0.2, m));
				}
			}
		}
	}

	printf("random scene made!\r\n");

	int m1 = HittableList_addMat(&world, MakeDielectric(1.5));
	HittableList_add(&world, MakeSphere(point3(0, 1, 0), 1.0, m1));

	int m2 = HittableList_addMat(&world, MakeLambertian(color(0.4, 0.2, 0.1)));
	HittableList_add(&world, MakeSphere(point3(-4, 1, 0), 1.0, m2));

	int m3 = HittableList_addMat(&world, MakeMetal(color(0.7, 0.6, 0.5), 0.0));
	HittableList_add(&world, MakeSphere(point3(4, 1, 0), 1.0, m3));

	printf("world:\r\n");
	HittableList_print(&world, "\t");

	Vector3 lookfrom = point3(13, 2, 3);
	Vector3 lookat = point3(0, 0, 0);

	world.camera = MakeCamera(
		lookfrom, // origin
		lookat, // look at
		vec3(0, 1, 0), // up
		20, // fov
		0.1, // aperture
		Vector3Length(Vector3Subtract(lookfrom, lookat)), // we focus on the point we're looking at
		0, // image width
		0 // image height
	);

	return world;
}

#endif
