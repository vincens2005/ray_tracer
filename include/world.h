#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>

typedef struct {
	double radius;
	Vector3 center;
} Sphere;

typedef struct {
	Vector3 p;
	Vector3 normal;
	double t;
	bool front_face;
} HitRecord;

union HittableObject {
	Sphere sphere;
};

typedef struct {
	union HittableObject object;
	bool (*hit)(union HittableObject o, const Ray r, double t_min, double t_max, HitRecord *rec);
	void (*print)(union HittableObject o);
} Hittable;


void set_face_normal(HitRecord *rec, const Ray r, const Vector3 outward_normal) {
	rec->front_face = dot(r.direction, outward_normal) < 0;
	rec->normal = rec->front_face ? outward_normal : Vector3Negate(outward_normal);
}

bool Sphere_hit(union HittableObject o, const Ray r, double t_min, double t_max, HitRecord *rec) {
	Sphere s = o.sphere;
	Vector3 oc = Vector3Subtract(r.position, s.center);
	double a = Vector3LengthSqr(r.direction);
	double half_b = dot(oc, r.direction);
	double c = Vector3LengthSqr(oc) - s.radius*s.radius;
	double discriminant = half_b*half_b - a*c;

	if (discriminant < 0) return false;
	double sqrtd = sqrt(discriminant);

	double t = (-half_b - sqrtd) / a; // goofy ahh quadratic formula
	// Find the nearest t that lies in the acceptable range.
	if (t <= t_min || t >= t_max) {
		t = (-half_b + sqrtd) / a;
		if (t <= t_min || t >= t_max) {
			return false;
		}
	}

	rec->t = t;
	rec->p = Ray_at(r, rec->t);

	Vector3 outward_normal = Vector3Scale(Vector3Subtract(rec->p, s.center), 1.0 / s.radius);
	set_face_normal(rec, r, outward_normal); // if the ray is inside the sphere the normal should be inverted

	return true;
}

void Sphere_print(union HittableObject o) {
	Sphere s = o.sphere;
	printf("Sphere (%2f, %2f, %2f) radius %2f\r\n", s.center.x, s.center.y, s.center.z, s.radius);
}

Hittable MakeSphere(Vector3 center, double radius) {
	Hittable s;
	s.hit = Sphere_hit;
	s.print = Sphere_print;
	s.object.sphere = (Sphere){radius, center};
	return s;
}

typedef struct {
	Hittable* objects;
	int len;
} HittableList;

void HittableList_clear(HittableList* list) {
	free(list->objects);
	list->objects = (Hittable*)malloc(sizeof(Hittable));
	list->len = 1;
}

void HittableList_add(HittableList* list, Hittable obj) {
	list->len++;
	list->objects = (Hittable*)realloc(list->objects, sizeof(Hittable) * list->len);
	list->objects[list->len - 1] = obj;
}

HittableList MakeHittableList() {
	return (HittableList){
		 (Hittable*)malloc(sizeof(Hittable)),
		 0
	};
}

bool HittableList_hit(HittableList* l, const Ray r, double t_min, double t_max, HitRecord* rec) {
	HitRecord temp_rec;
	bool hit_anything = false;
	double closest_so_far = t_max;

	for (int i = 0; i < l->len; i++) {
		if (l->objects[i].hit(l->objects[i].object, r, t_min, closest_so_far, &temp_rec)) {
			hit_anything = true;
			closest_so_far = temp_rec.t;
			*rec = temp_rec;
		}
	}
	return hit_anything;
}

void HittableList_print(HittableList* l, char* tabulation) {
	printf("%slist of length %d\r\n", tabulation, l->len);
	for (int i = 0; i < l->len; i++) {
		printf("%s\t", tabulation);
		l->objects[i].print(l->objects[i].object);
	}
}