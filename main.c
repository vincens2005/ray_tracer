#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "include/raylib.h"
#include "include/raymath.h"
#include "include/utils.h"

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
	printf("checking hits in the world\r\n");
	if (HittableList_hit(&world, r, 0, INFINITY, &rec)) {
		return Vector3Scale(Vector3Add(rec.normal, color(1,1,1)), 0.5);
	}
	Vector3 unit_direction = UnitVector(r.direction);
	double t = 0.5 * (unit_direction.y + 1.0f);
	return Vector3Add(Vector3Scale(Vector3One(), 1.0f - t), Vector3Scale(color(0.5, 0.7, 1.0), t));
}

void draw_image() {
	// image
	int image_width = GetScreenWidth();
	int image_height = GetScreenHeight();

	// world
	printf("making world\r\n");
	HittableList world = MakeHittableList();
	printf("making balls\r\n");
	HittableList_add(&world, MakeSphere(point3(0, 0, -1), 0.5));
	HittableList_add(&world, MakeSphere(point3(0, -100.5, -1), 100));

	printf("balls initialized\r\n");

	// camera
	double viewport_height = 2.0f;
	double viewport_width = viewport_height * ((double)image_width / (double)image_height);
	double focal_length = 1.0f;

	Vector3 origin = Vector3Zero();
	Vector3 horizontal = vec3(viewport_width, 0, 0);
	Vector3 vertical = vec3(0, viewport_height, 0);

	Vector3 lower_left_corner = Vector3Subtract(
		Vector3Subtract(origin, Vector3Scale(horizontal, 0.5f)),
		Vector3Subtract(Vector3Scale(vertical, 0.5f), vec3(0, 0, focal_length))
	);

	for (int j = image_height - 1; j >= 0; j--) {
		for (int i = 0; i < image_width; i++) {
			double u = (double)i / (image_width - 1);
			double v = (double)j / (image_height - 1);

			Ray r = (Ray){origin, Vector3Subtract(
				Vector3Add(
					lower_left_corner,
					Vector3Add(
						Vector3Scale(horizontal, u),
						Vector3Scale(vertical, v)
				)),
				origin
			)};

			Color color = Vector3ToColor(ray_color(r, world));

			DrawPixel(i, image_height - j, color);
		}
	}
}

int main() {
	printf("balls\r\n");
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(640, 480, "Hello World!!");

	SetTargetFPS(60);

	while (!WindowShouldClose()) {
		BeginDrawing();
			ClearBackground(BLACK);
			draw_image();
			DrawFPS(10, 10);
		EndDrawing();
	}
	CloseWindow();

	return 0;
}
