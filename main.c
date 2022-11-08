#include <stdio.h>
#include "include/raylib.h"
#include "include/raymath.h"
#include "include/utils.h"

Vector3 ray_color(Ray r) {
	Vector3 unit_direction = UnitVector(r.direction);
	double t = 0.5 * (unit_direction.y + 1.0f);
	return Vector3Add(Vector3Scale(Vector3One(), 1.0f - t), Vector3Scale(color(0.5, 0.7, 1.0), t));
}

void draw_image() {
	int image_width = GetScreenWidth();
	int image_height = GetScreenHeight();

	for (int j = image_height - 1; j >= 0; j--) {
		for (int i = 0; i < image_width; i++) {
			double r = (double)i / (image_width - 1);
			double g = (double)j / (image_height - 1);
			double b = 0.25;

			Color color = {255.999f * r, 255.999f * g, 255.999f * b, 255};

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
