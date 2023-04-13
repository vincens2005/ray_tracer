#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "raylib.h"
#include "raymath.h"
#include "utils.h"
#include "hittable_list.h"
#include "camera.h"
#include "scenes.h"

int samples_per_pixel = 1000;
int max_bounces = 25;

Vector3 ray_color(Ray r, HittableList* world, int depth) {
	HitRecord rec;
	if (depth <= 0) {
		return color(0, 0, 0);
	}
	if (HittableList_hit(world, r, 0.001, INFINITY, &rec)) {
		Ray scattered;
		Vector3 attenuation = color(0, 0, 0);
		if (world->materials[rec.mat_i].scatter(world->materials[rec.mat_i].object, r, &rec, &attenuation, &scattered)) {
			return Vector3Multiply(attenuation, ray_color(scattered, world, depth - 1));
		}
		return attenuation;
	}
	// return color(0,0,0); // black sky
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

	srand(time(NULL));

	// make world
	printf("making world\r\n");

	HittableList world = sexy_scene();

	printf("got world!\r\n");

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

		// t for zoom in; r for zoom out
		double fov_delta = 0;
		if (IsKeyDown(KEY_T)) {
			fov_delta -= 1;
		}
		if (IsKeyDown(KEY_R)) {
			fov_delta += 1;
		}

		// o for wider aperture, i for smaller
		double aperture_delta = 0.0;
		if (IsKeyDown(KEY_I)) {
			aperture_delta -= 0.05;
		}
		if (IsKeyDown(KEY_O)) {
			aperture_delta += 0.05;
		}

		// x for closer focus; z for farther
		double focusdist_delta = 0.0;
		if (IsKeyDown(KEY_X)) {
			focusdist_delta -= 0.1;
		}
		if (IsKeyDown(KEY_C)) {
			focusdist_delta += 0.1;
		}

		Vector3 camera_movement_vector = Vector3Add(
			Vector3Add(
				Vector3Scale((Vector3){world.camera.w.x, 0, world.camera.w.z}, camera_delta.z),
				Vector3Scale((Vector3){world.camera.u.x, 0, world.camera.u.z}, camera_delta.x)),
				vec3(0, camera_delta.y, 0)
		);

		camera_lookat_delta = Vector3Add(
			Vector3Add(
				Vector3Scale(world.camera.w, camera_lookat_delta.z),
				Vector3Scale(world.camera.u, camera_lookat_delta.x)),
				Vector3Scale(world.camera.v, camera_lookat_delta.y)
		);
		camera_lookat_delta = Vector3Add(camera_movement_vector, camera_lookat_delta);
		if (Vector3Length(camera_delta) != 0 || Vector3Length(camera_lookat_delta) != 0 || fov_delta != 0 || aperture_delta != 0 || focusdist_delta != 0) {
			Camera_update(
				&(world.camera),
				Vector3Add(world.camera.origin, camera_movement_vector),
				Vector3Add(world.camera.lookat, camera_lookat_delta),
				world.camera.vup,
				world.camera.vfov + fov_delta,
				world.camera.aperture + aperture_delta,
				world.camera.focus_dist + focusdist_delta,
				pic.width, pic.height
			);
			world.changed = true;
		}

		printf("rendering sample %d\r\n", pic.sample_count + 1);

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
