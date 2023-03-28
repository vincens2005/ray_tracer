#ifndef UTILS
#define UTILS

#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

#define color(r,g,b) ((Vector3){r,g,b})
#define vec3(x,y,z) ((Vector3){x,y,z})
#define point3(x,y,z) ((Vector3){x,y,z})
#define dot(a,b) (a.x*b.x + a.y*b.y + a.z*b.z)
#define pi (double)3.1415926535897932385
#define degrees_to_radians(d) (d * pi / 180.0f)
#define ndigits(i) ((int)(i != 0 ? floor(log10(abs(i))) + 1 : 1))
#define ray(o, v) ((Ray){o, v})
#define printvector(v) (printf("(%f, %f, %f)", v.x, v.y, v.z))
#define random_color() Vector3Random()
#define vec2arr(v) {v.x, v.y, v.z} // this is a little iffy

typedef struct {
	int width;
	int height;
	int sample_count;
	Vector3** color;
} Picture;

Picture MakePicture(int width, int height) {
	Picture p = {
		width,
		height,
		0
	};

	p.color = (Vector3**)malloc(width * sizeof(Vector3*));

	for (int i = 0; i < width; i++) {
		p.color[i] = (Vector3*)malloc(height * sizeof(Vector3));
	}

	return p;
}

Vector3 Picture_at(Picture *p, int u, int v) {
	return p->color[u][v];
}

void Picture_set(Picture *p, int u, int v, Vector3 color) {
	p->color[u][v] = color;
}

void Picture_free(Picture *p) {
	for (int i = 0; i < p->width; i++) {
		free(p->color[i]);
	}

	free(p->color);
	p->sample_count = 0;
	p->width = 0.0;
	p->height = 0.0;
}


Vector3 Ray_at(Ray r, double t) {
	return Vector3Add(r.position, Vector3Scale(r.direction, t));
}

Vector3 UnitVector(Vector3 v) {
	return Vector3Scale(v, 1.0f/Vector3Length(v));
}

double clamp(double x, double min, double max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

Color Vector3ToColor(Vector3 v, double s) {
	v = Vector3Scale(v, s);
	v.x = sqrt(v.x);
	v.y = sqrt(v.y);
	v.z = sqrt(v.z);
	return (Color){
		(int)(clamp(v.x, 0.0, 0.999) * 256),
		(int)(clamp(v.y, 0.0, 0.999) * 256),
		(int)(clamp(v.z, 0.0, 0.999) * 256),
		255
	};
}

double random_double1() {
    // Returns a random real in [0,1).
    return rand() / (RAND_MAX + 1.0);
}

double random_double(double min, double max) {
    // Returns a random real in [min,max).
    return min + (max-min)*random_double1();
}

Vector3 Vector3Random() {
	return vec3((float)random_double1(), (float)random_double1(), (float)random_double1());
}

Vector3 Vector3RandRange(double min, double max) {
	return vec3((float)random_double(min, max), (float)random_double(min, max), (float)random_double(min, max));
}

Vector3 random_in_unit_sphere() {
	while (true) {
		Vector3 p = Vector3RandRange(-1, 1);
		if (Vector3LengthSqr(p) >= 1) continue;
		return p;
	}
}

Vector3 random_in_unit_disk() {
	while (true) {
		Vector3 p = vec3((float)random_double(-1, 1), (float)random_double(-1, 1), 0);
		if (Vector3LengthSqr(p) >= 1) continue;
		return p;
	}
}

Vector3 random_unit_vector() {
    return UnitVector(random_in_unit_sphere());
}

int randint(int min, int max) {
	return (rand() / (1 + RAND_MAX)) * (max - min) + min;
}

double reflectance(double cosine, double ref_index) { // schlick approximation
	double r0 = (1 - ref_index) / (1 + ref_index);
	r0 = r0 * r0;
	return r0 + (1 - r0) * pow((1 - cosine), 5);
}
#endif
