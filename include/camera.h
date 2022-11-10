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
	Vector3 lookat;
	Vector3 vup;
	double vfov;
} Cam;

Ray Camera_getRay(Cam c, double u, double v) {
	return ray(c.origin, Vector3Add(
		c.lower_left_corner,
		Vector3Subtract(
			Vector3Add(
				Vector3Scale(c.horizontal, u),
				Vector3Scale(c.vertical, v)
			),
			c.origin
	)));
}

void Camera_update(Cam *c, Vector3 origin, Vector3 lookat, Vector3 vup, double vfov, int image_width, int image_height) {
	double theta = degrees_to_radians(vfov);
	double h = tan(theta / 2);

	float viewport_height = 2.0 * h;
	float viewport_width = viewport_height * ((double)image_width / (double)image_height);
	float focal_length = 1.0f;

	c->origin = origin;
	c->lookat = lookat;
	c->vup = vup;

	Vector3 w = UnitVector(Vector3Subtract(origin, lookat));
	Vector3 u = UnitVector(Vector3CrossProduct(vup, w));
	Vector3 v = Vector3CrossProduct(w, u);

	c->horizontal = Vector3Scale(u, viewport_width);
	c->vertical = Vector3Scale(v, viewport_height);
	c->vfov = vfov;

	c->lower_left_corner = Vector3Subtract(
		Vector3Subtract(
			Vector3Subtract(c->origin, Vector3Scale(c->horizontal, 0.5f)),
			Vector3Subtract(Vector3Scale(c->vertical, 0.5f), vec3(0, 0, -focal_length))),
		w
	);
}

Cam MakeCamera(Vector3 origin, Vector3 lookat, Vector3 vup, double vfov, int image_width, int image_height) {
	Cam c;

	Camera_update(&c, origin, lookat, vup, vfov, image_width, image_height);

	return c;
}

#endif
