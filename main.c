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
		Camera_update(&(world->camera), world->camera.origin, world->camera.lookat, world->camera.vup, world->camera.vfov, world->camera.aperture, world->camera.focus_dist, image_width, image_height);
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
	Mat ground = MakeLambertian(color(0.2, 0.6, 0.1));

	// make world
	printf("making world\r\n");
	HittableList world = MakeHittableList();
	printf("making balls\r\n");
	HittableList_add(&world, MakeSphere(point3(0, -100.5, -1), 100, &ground));

	HittableList_add(&world, MakeSphere(point3(-1, 0, -1), 0.5, &glass));
	HittableList_add(&world, MakeSphere(point3(0, 0, -1), 0.5, &lambertiangray));
	HittableList_add(&world, MakeSphere(point3(1, 0, -1), 0.5, &redmetal));

	HittableList_add(&world, MakeSphere(point3(0.1, 1, -1.5), 0.34, &metal));

	printf("balls initialized\r\n\tworld:\r\n");
	HittableList_print(&world, "\t");

	Vector3 lookfrom = vec3(3, 3, 2);
	Vector3 lookat = vec3(0, 0, -1);

	world.camera = MakeCamera(
		lookfrom, // origin
		lookat, // look at
		vec3(0, 1, 0), // up
		50, // fov
		0.0, // aperture
		Vector3Length(Vector3Subtract(lookfrom, lookat)), // we focus on the point we're looking at
		0, // image width
		0 // image height
	);

	Picture pic = MakePicture(0, 0);

	while (!WindowShouldClose()) {
		bool screenshotting = IsKeyReleased(80);

		// camera movement
		// wasd + q for up and z for down
		Vector3 camera_delta = vec3(0,0,0);
		if (IsKeyDown(KEY_W)) {
			camera_delta.z -= 0.1;
		}
		if (IsKeyDown(KEY_S)) {
			camera_delta.z += 0.1;
		}
		if (IsKeyDown(KEY_A)) {
			camera_delta.x -= 0.1;
		}
		if (IsKeyDown(KEY_D)) {
			camera_delta.x += 0.1;
		}
		if (IsKeyDown(KEY_Q)) {
			camera_delta.y += 0.1;
		}
		if (IsKeyDown(KEY_Z)) {
			camera_delta.y -= 0.1;
		}

		Vector3 camera_lookat_delta = vec3(0, 0, 0);
		// arrow keys
		if (IsKeyDown(KEY_DOWN)) {
			camera_lookat_delta.y -= 0.1;
		}
		if (IsKeyDown(KEY_UP)) {
			camera_lookat_delta.y += 0.1;
		}
		if (IsKeyDown(KEY_LEFT)) {
			camera_lookat_delta.x -= 0.1;
		}
		if (IsKeyDown(KEY_RIGHT)) {
			camera_lookat_delta.x += 0.1;
		}

		double fov_delta = 0;
		if (IsKeyDown(KEY_T)) {
			fov_delta -= 1;
		}
		if (IsKeyDown(KEY_R)) {
			fov_delta += 1;
		}

		Vector3 camera_movement_vector = Vector3Add(
			Vector3Add(
				Vector3Scale(world.camera.w, camera_delta.z),
				Vector3Scale(world.camera.u, camera_delta.x)),
				Vector3Scale(world.camera.v, camera_delta.y)
		);

		camera_lookat_delta = Vector3Add(
			Vector3Add(
				Vector3Scale(world.camera.w, camera_lookat_delta.z),
				Vector3Scale(world.camera.u, camera_lookat_delta.x)),
				Vector3Scale(world.camera.v, camera_lookat_delta.y)
		);
		camera_lookat_delta = Vector3Add(camera_movement_vector, camera_lookat_delta);
		if (Vector3Length(camera_delta) != 0 || Vector3Length(camera_lookat_delta) != 0 || fov_delta != 0) {
			Camera_update(
				&(world.camera),
				Vector3Add(world.camera.origin, camera_movement_vector),
				Vector3Add(world.camera.lookat, camera_lookat_delta),
				world.camera.vup,
				world.camera.vfov + fov_delta,
				world.camera.aperture,
				world.camera.focus_dist,
				pic.width, pic.height
			);
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
