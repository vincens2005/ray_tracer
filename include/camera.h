#ifndef CAMERA
#define CAMERA

#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "utils.h"

typedef struct {
	Vector3 origin;
	Vector3 lower_left_corner;
	Vector3 horizontal;
	Vector3 vertical;
} Cam;

Ray Camera_getRay(Cam c, double u, double v) {
	return ray(c.origin, Vector3Add(
		c.lower_left_corner,
		Vector3Add(
			Vector3Scale(c.horizontal, u),
			Vector3Scale(c.vertical, v)
	)));
}

void Camera_update(Cam *c, Vector3 origin, int image_width, int image_height) {
	float viewport_height = 2.0f;
	float viewport_width = viewport_height * ((double)image_width / (double)image_height);
	float focal_length = 1.0f;
	c->origin = origin;
	c->horizontal = vec3(viewport_width, 0, 0);
	c->vertical = vec3(0, viewport_height, 0);

	c->lower_left_corner = Vector3Subtract(
		Vector3Subtract(c->origin, Vector3Scale(c->horizontal, 0.5f)),
		Vector3Subtract(Vector3Scale(c->vertical, 0.5f), vec3(0, 0, -focal_length))
	);
}

Cam MakeCamera(Vector3 origin, int image_width, int image_height) {
	Cam c;

	Camera_update(&c, origin, image_width, image_height);

	return c;
}

#endif
