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
	double aperture;
	double focus_dist;
	Vector3 u, v, w;
	double lens_radius;
} Cam;

Ray Camera_getRay(Cam c, double s, double t) {
	Vector3 rd = Vector3Scale(random_in_unit_disk(), c.lens_radius);
	Vector3 offset = Vector3Add(Vector3Scale(c.u, rd.x), Vector3Scale(c.v, rd.y));

	Vector3 ray_direction = Vector3Add(c.lower_left_corner, Vector3Scale(c.horizontal, s));
	ray_direction = Vector3Add(ray_direction, Vector3Scale(c.vertical, t));
	ray_direction = Vector3Subtract(ray_direction, c.origin);
	ray_direction = Vector3Subtract(ray_direction, offset);

	return ray(Vector3Add(c.origin, offset), ray_direction);
}

void Camera_update(Cam *c, Vector3 origin, Vector3 lookat, Vector3 vup, double vfov, double aperture, double focus_dist, int image_width, int image_height) {
	double theta = degrees_to_radians(vfov);
	double h = tan(theta / 2);

	float viewport_height = 2.0 * h;
	float viewport_width = viewport_height * ((double)image_width / (double)image_height);
	// float focal_length = 1.0f;

	c->origin = origin;
	c->lookat = lookat;
	c->vup = vup;
	c->focus_dist = focus_dist;
	c->aperture = aperture;

	c->w = UnitVector(Vector3Subtract(origin, lookat));
	c->u = UnitVector(Vector3CrossProduct(vup, c->w));
	c->v = Vector3CrossProduct(c->w, c->u);

	c->horizontal = Vector3Scale(c->u, viewport_width * focus_dist);
	c->vertical = Vector3Scale(c->v, viewport_height * focus_dist);
	c->vfov = vfov;

	c->lower_left_corner = Vector3Subtract(
		Vector3Subtract(c->origin, Vector3Scale(c->horizontal, 0.5)),
		Vector3Subtract(Vector3Scale(c->vertical, 0.5), Vector3Scale(c->w, -focus_dist))
	);

	c->lens_radius = aperture / 2;
}

Cam MakeCamera(Vector3 origin, Vector3 lookat, Vector3 vup, double vfov, double aperture, double focus_dist, int image_width, int image_height) {
	Cam c;

	Camera_update(&c, origin, lookat, vup, vfov, aperture, focus_dist, image_width, image_height);

	return c;
}

#endif
