#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#define color(r,g,b) ((Vector3){r,g,b})
#define vec3(x,y,z) ((Vector3){x,y,z})
#define point3(x,y,z) ((Vector3){x,y,z})
#define dot(a,b) (a.x*b.x + a.y*b.y + a.z*b.z)
#define pi (double)3.1415926535897932385
#define degrees_to_radians(d) (d * pi / 180.0f)


Vector3 Ray_at(Ray r, double t) {
	return Vector3Add(r.position, Vector3Scale(r.direction, t));
}

Vector3 UnitVector(Vector3 v) {
	return Vector3Scale(v, 1.0f/Vector3Length(v));
}

Color Vector3ToColor(Vector3 v) {
	return (Color){
		(int)(v.x * 255.999),
		(int)(v.y * 255.999),
		(int)(v.z * 255.999),
		255
	};
}


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

	printf("checking if we hit a sphere\r\n");

	if (discriminant < 0) return false;
	double sqrtd = sqrt(discriminant);

	double root = (-half_b - sqrtd) / a;

		// Find the nearest root that lies in the acceptable range.
		if (root < t_min || t_max < root) {
			root = (-half_b + sqrtd) / a;
			if (root < t_min || t_max < root)
					return false;
	}
	printf("oh dear\r\n");
	printf("root: %9f\r\n", root);
	printf("t: %9f\r\n", rec->t);
	rec->t = root;

	rec->p = Ray_at(r, rec->t);
	Vector3 outward_normal = Vector3Scale(Vector3Subtract(rec->p, s.center), 1.0f / s.radius);
	set_face_normal(rec, r, outward_normal); // if the ray is inside the sphere the normal should be inverted

	return true;
}

Hittable MakeSphere(Vector3 center, double radius) {
	Hittable s;
	s.hit = Sphere_hit;
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
	HitRecord *temp_rec;
	bool hit_anything = false;
	double closest_so_far = t_max;

	for (int i = 0; i < l->len; i++) {
		if (l->objects[i].hit(l->objects[i].object, r, t_min, closest_so_far, temp_rec)) {
			printf("whoa we hit a thing!!!\r\n");
			hit_anything = true;
			closest_so_far = temp_rec->t;
			rec = temp_rec;
		}
	}
	return hit_anything;
}

void HittableList_print(HittableList* list) {
	printf("list\r\n");
	for (int i = 0; i < list->len; i++) {
		printf("\t object;\r\n");
	}
	printf("done");
}
