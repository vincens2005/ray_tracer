#include "raylib.h"
#include "raymath.h"

#define color(r,g,b) ((Vector3){r,g,b})
#define vec3(x,y,z) ((Vector3){x,y,z})
#define point3(x,y,z) ((Vector3){x,y,z})

Vector3 Ray_at(Ray r, double t) {
	return Vector3Add(r.position, Vector3Scale(r.direction, t));
}

Vector3 UnitVector(Vector3 v) {
	return Vector3Scale(v, 1.0f/Vector3Length(v));
}
