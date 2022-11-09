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
	
} Picture;


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

double random_double(double min, double max) {
    // Returns a random real in [min,max)
    srandom(time(0));
    return min + (max-min) * (random() / (RAND_MAX + 1.0));
}

