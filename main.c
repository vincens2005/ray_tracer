#include <stdio.h>
#include <math.h>
#include "include/raylib.h"
#include "include/raymath.h"
#include "include/utils.h"

double hit_sphere(const Vector3 center, double radius, const Ray r) {
	Vector3 oc = Vector3Subtract(r.position, center);
	double a = dot(r.direction, r.direction);
	double b = 2.0 * dot(oc, r.direction)	;
	double c = dot(oc, oc) - radius*radius;
	double discriminant = b*b - 4*a*c;
	if (discriminant < 0) {
		return -1.0;
	}
	return abs((-b - sqrtf(discriminant)) / (2.0f*a));
}

Vector3 ray_color(Ray r) {
	double t = hit_sphere(point3(0, 0, -1), 0.5, r);
	if (t > 0.0f) {
		// printf("t: %9f\r\n", t);
		Vector3 N = UnitVector(Vector3Subtract(Ray_at(r, t), vec3(0, 0, -1)));
		return Vector3Scale(Vector3AddValue(N, 1), 0.5);
	}
	Vector3 unit_direction = UnitVector(r.direction);
	t = 0.5 * (unit_direction.y + 1.0f);
	return Vector3Add(Vector3Scale(Vector3One(), 1.0f - t), Vector3Scale(color(0.5, 0.7, 1.0), t));
}

void draw_image() {
	int image_width = GetScreenWidth();
	int image_height = GetScreenHeight();

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

			Color color = Vector3ToColor(ray_color(r));

			DrawPixel(i, image_height - j, color);
		}
	}
}

int main() {
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
