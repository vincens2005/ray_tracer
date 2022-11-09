#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "include/raylib.h"
#include "include/raymath.h"
#include "include/utils.h"
#include "include/world.h"
#include "include/camera.h"

int samples_per_pixel = 100;

double hit_sphere(const Vector3 center, double radius, const Ray r) {
	Vector3 oc = Vector3Subtract(r.position, center);
	double a = Vector3LengthSqr(r.direction);
	double half_b = dot(oc, r.direction);
	double c = Vector3LengthSqr(oc) - radius*radius;
	double discriminant = half_b*half_b - a*c;
	if (discriminant < 0) {
		return -1.0;
	}
	return fabs(-half_b - sqrt(discriminant) / a);
}

Vector3 ray_color(Ray r, HittableList world) {
	HitRecord rec;
	if (HittableList_hit(&world, r, 0, INFINITY, &rec)) {
		return Vector3Scale(Vector3Add(rec.normal, color(1,1,1)), 0.5);
	}
	Vector3 unit_direction = UnitVector(r.direction);
	double t = 0.5 * (unit_direction.y + 1.0f);
	return Vector3Add(Vector3Scale(Vector3One(), 1.0f - t), Vector3Scale(color(0.5, 0.7, 1.0), t));
}

HittableList make_world() {
	printf("making world\r\n");
	HittableList world = MakeHittableList();
	printf("making balls\r\n");
	HittableList_add(&world, MakeSphere(point3(0, -100.5, -1), 100));
	HittableList_add(&world, MakeSphere(point3(0, 0, -1), 0.5f));
	printf("balls initialized\r\n\tworld:\r\n");
	HittableList_print(&world, "\t");
	return world;
}

void draw_image(HittableList world, Picture* pic) {
	// image
	int image_width = GetScreenWidth();
	int image_height = GetScreenHeight();

	if (pic->width != image_width || pic->height != image_height) {
		Picture_free(pic);
		*pic = MakePicture(image_width, image_height);
	}

	pic->sample_count++;
	Cam cam = MakeCamera(image_width, image_height);


	for (int j = image_height - 1; j >= 0; j--) {
		for (int i = 0; i < image_width; i++) {
			if (pic->sample_count > samples_per_pixel) {
				DrawPixel(i, image_height - j, Vector3ToColor(Picture_at(pic, i, j), 1.0 / pic->sample_count));
			}
			double u = (i + random_double1()) / (image_width - 1);
			double v = (j + random_double1()) / (image_height - 1);

			Ray r = Camera_getRay(cam, u, v);

			Vector3 color = ray_color(r, world);

			if (pic->sample_count > 1)
				color = Vector3Add(color, Picture_at(pic, i, j));

			Picture_set(pic, i, j, color);
			// printf("expected value: %f. Actual value: %f\r\n", color.z, test.z);

			DrawPixel(i, image_height - j, Vector3ToColor(color, 1.0 / pic->sample_count));
		}
	}
}

int main() {
	printf("balls\r\n");
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(640, 480, "Hello World!!");

	SetTargetFPS(60);

	HittableList world = make_world();
	Picture pic = MakePicture(0, 0);

	while (!WindowShouldClose()) {
		BeginDrawing();
			ClearBackground(BLACK);
			draw_image(world, &pic);
			DrawFPS(10, 10);
		EndDrawing();
	}
	CloseWindow();

	return 0;
}
