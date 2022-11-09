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

typedef struct {
	int width;
	int height;
	int sample_count;
	Vector3* color;
} Picture;

Picture MakePicture(int width, int height) {
	printf("making p\r\n");
	Picture p = {
		width,
		height,
		0
	};

	p.color = (Vector3*)malloc(width * height * sizeof(Vector3));

	return p;
}

Vector3 Picture_at(Picture *p, int u, int v) {
	return p->color[v * p->height + u];
}

void Picture_set(Picture *p, int u, int v, Vector3 color) {
	p->color[v * p->height + u] = color;
}

void Picture_free(Picture *p) {
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
