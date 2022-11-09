#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "include/raylib.h"
#include "include/raymath.h"
#include "include/utils.h"
#include "include/world.h"
#include "include/camera.h"

int samples_per_pixel = 100;
int max_bounces = 25;

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

Vector3 ray_color(Ray r, HittableList* world, int depth) {
	HitRecord rec;
	if (depth <= 0) {
		return color(0, 0, 0);
	}
	if (HittableList_hit(world, r, 0.001, INFINITY, &rec)) {
		Ray scattered;
		Vector3 attenuation;
		if (rec.mat_ptr->scatter(rec.mat_ptr->object, r, &rec, &attenuation, &scattered)) {
			return Vector3Multiply(attenuation, ray_color(scattered, world, depth - 1));
		}
		return color(0, 0, 0);
	}
	Vector3 unit_direction = UnitVector(r.direction);
	double t = 0.5 * (unit_direction.y + 1.0f);
	return Vector3Add(Vector3Scale(Vector3One(), 1.0f - t), Vector3Scale(color(0.5, 0.7, 1.0), t));
}

void draw_image(HittableList* world, Picture* pic) {
	// image
	int image_width = GetScreenWidth();
	int image_height = GetScreenHeight();

	if (pic->width != image_width || pic->height != image_height || world->changed) {
		Picture_free(pic);
		*pic = MakePicture(image_width, image_height);
		Camera_update(&(world->camera), world->camera.origin, image_width, image_height);
		world->changed = false; // acknowledge change
	}
	if (pic->sample_count < samples_per_pixel)
		pic->sample_count++;



	for (int j = image_height - 1; j >= 0; j--) {
		for (int i = 0; i < image_width; i++) {
			if (pic->sample_count >= samples_per_pixel) {
				DrawPixel(i, image_height - j, Vector3ToColor(Picture_at(pic, i, j), 1.0 / pic->sample_count));
				continue;
			}

			double u = (i + random_double1()) / (image_width - 1);
			double v = (j + random_double1()) / (image_height - 1);

			Ray r = Camera_getRay(world->camera, u, v);

			Vector3 color = ray_color(r, world, max_bounces);

			if (pic->sample_count > 1)
				color = Vector3Add(color, Picture_at(pic, i, j));

			Picture_set(pic, i, j, color);

			DrawPixel(i, image_height - j, Vector3ToColor(color, 1.0 / pic->sample_count));
		}
	}
}

int main() {
	printf("balls\r\n");
	SetConfigFlags(FLAG_WINDOW_RESIZABLE);
	InitWindow(640, 480, "Hello World!!");

	SetTargetFPS(60);

	// make materials
	printf("making materials\r\n");
	Mat lambertiangray = MakeLambertian(color(0.7, 0.7, 0.7));
	Mat redmetal = MakeMetal(color(0.7, 0.3, 0.3), 0.05);
	Mat metal = MakeMetal(color(0.8, 0.8, 0.8), 0.3);
	Mat glass = MakeDielectric(1.51);

	// make world
	printf("making world\r\n");
	HittableList world = MakeHittableList();
	printf("making balls\r\n");
	HittableList_add(&world, MakeSphere(point3(0, -100.5, -1), 100, &lambertiangray));

	HittableList_add(&world, MakeSphere(point3(-1, 0, -1), 0.5, &glass));
	HittableList_add(&world, MakeSphere(point3(0, 0, -1), 0.5, &lambertiangray));
	HittableList_add(&world, MakeSphere(point3(1, 0, -1), 0.5, &redmetal));
	
	HittableList_add(&world, MakeSphere(point3(0.1, 0.6, -1), 0.34, &metal));

	printf("balls initialized\r\n\tworld:\r\n");
	HittableList_print(&world, "\t");
	world.camera = MakeCamera(Vector3Zero(), 0, 0);

	Picture pic = MakePicture(0, 0);

	while (!WindowShouldClose()) {
		bool screenshotting = IsKeyReleased(80);

		// camera movement
		Vector3 camera_delta = vec3(0,0,0);
		if (IsKeyDown(87)) {
			camera_delta.z -= 0.1;
		}
		if (IsKeyDown(83)) {
			camera_delta.z += 0.1;
		}
		if (IsKeyDown(65)) {
			camera_delta.x -= 0.1;
		}
		if (IsKeyDown(68)) {
			camera_delta.x += 0.1;
		}
		if (IsKeyDown(81)) {
			camera_delta.y += 0.1;
		}
		if (IsKeyDown(90)) {
			camera_delta.y -= 0.1;
		}

		if (Vector3Length(camera_delta) != 0 ) {
			Camera_update(&(world.camera), Vector3Add(world.camera.origin, camera_delta), pic.width, pic.height);
			world.changed = true;
		}

		BeginDrawing();
			ClearBackground(BLACK);
			draw_image(&world, &pic);

			if (!screenshotting) {
				DrawFPS(10, 10);
				char sample[ndigits(pic.sample_count) + 7];
				sprintf(sample, "sample %d", pic.sample_count);
				DrawText(sample, 10, 30, 20, WHITE);

				if (pic.sample_count >= samples_per_pixel) {
					DrawText("rendering done!", 10, 50, 20, DARKGREEN);
				}
			}

		EndDrawing();

		if (screenshotting) {
			TakeScreenshot("render.png");
		}
	}
	CloseWindow();

	return 0;
}
