#ifndef WORLD
#define WORLD
#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "utils.h"
#include "camera.h"

typedef struct Mat Mat;

typedef struct {
	Vector3 p;
	Vector3 normal;
	double t;
	bool front_face;
	int mat_i;
} HitRecord;

// object type definitions
typedef struct {
	double radius;
	Vector3 center;
	int mat_i;
} Sphere;

typedef struct {
	Vector3 minimum;
	Vector3 maximum;
} Aabb;

typedef union {
	Sphere sphere;
	Aabb aabb;
} HittableObject;

typedef struct {
	HittableObject object;
	bool (*hit)(HittableObject o, const Ray r, double t_min, double t_max, HitRecord *rec);
	bool (*bounding_box)(HittableObject o, Aabb* output_box);
	void (*print)(HittableObject o);
} Hittable;


void set_face_normal(HitRecord *rec, const Ray r, const Vector3 outward_normal) {
	rec->front_face = dot(r.direction, outward_normal) < 0;
	rec->normal = rec->front_face ? outward_normal : Vector3Negate(outward_normal);
}

bool Sphere_hit(HittableObject o, const Ray r, double t_min, double t_max, HitRecord *rec) {
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
	rec->mat_i = s.mat_i;

	return true;
}

void Sphere_print(HittableObject o) {
	Sphere s = o.sphere;
	printf("Sphere (%2f, %2f, %2f) radius %2f material %d\r\n", s.center.x, s.center.y, s.center.z, s.radius, s.mat_i);
}

bool Sphere_boundingbox(HittableObject o, Aabb* output_box) {
	Sphere s = o.sphere;
	output_box->minimum = Vector3Subtract(s.center, vec3(s.radius, s.radius, s.radius));
	output_box->maximum = Vector3Add(s.center, vec3(s.radius, s.radius, s.radius));
	return true;
}

Hittable MakeSphere(Vector3 center, double radius, int material) {
	Hittable s;
	s.hit = Sphere_hit;
	s.print = Sphere_print;
	s.bounding_box = Sphere_boundingbox;
	s.object.sphere = (Sphere){radius, center};
	s.object.sphere.mat_i = material;
	return s;
}

void Aabb_print(HittableObject o) {
	Aabb a = o.aabb;
	printf("AABB min (%2f, %2f, %2f) max (%2f, %2f, %2f)\r\n", a.minimum.x, a.minimum.y, a.minimum.z, a.maximum.x, a.maximum.y, a.maximum.z);
}

bool Aabb_hit(HittableObject o, const Ray r, double t_min, double t_max, HitRecord *rec) {
	Aabb a = o.aabb;
	double min[3] = vec2arr(a.minimum);
	double max[3] = vec2arr(a.maximum);

	double r_origin[3] = vec2arr(r.position);
	double r_direction[3] = vec2arr(r.direction);

	for (int i = 0; i < 3; i++) {
		double t0 = fmin((min[i] - r_origin[i]) / r_direction[i],
			(max[i] - r_origin[i]) / r_direction[i]);
		double t1 = fmax((min[i] - r_origin[i]) / r_direction[i],
			(max[i] - r_origin[i]) / r_direction[i]);

		t_min = fmax(t0, t_min);
		t_max = fmin(t1, t_max);

		if (t_min >= t_max)
			return false;
	}
	return true;
}

Hittable MakeAabb(Vector3 minimum, Vector3 maximum) {
	Hittable a;
	a.hit = Aabb_hit;
	a.print = Aabb_print;
	a.object.aabb = (Aabb){minimum, maximum};
	return a;
}

// material types
typedef struct {
	Vector3 albedo;
} Lambertian;

typedef struct {
	Vector3 albedo;
	float roughness;
} Metal;

typedef struct {
	double ior;
} Dielectric;

typedef struct {
	Vector3 color;
	float brighness;
} Emissive;

typedef union {
	Lambertian lambertian;
	Metal metal;
	Dielectric dielectric;
	Emissive emissive;
} MaterialObject;


struct Mat {
	MaterialObject object;
	bool (*scatter)(MaterialObject o, const Ray r_in, HitRecord *rec, Vector3 *attenuation, Ray *scattered);
};

// material functions

bool Lambertian_scatter(MaterialObject o, const Ray r_in, HitRecord *rec, Vector3 *attenuation, Ray *scattered) {
	Lambertian l = o.lambertian;
	Vector3 scatter_direction = Vector3Add(rec->normal, random_unit_vector());

	if (Vector3Equals(scatter_direction, vec3(0,0,0))) {
		scatter_direction = rec->normal;
	}

	*scattered = ray(rec->p, scatter_direction);
	*attenuation = l.albedo;
	return true;
}

bool Metal_scatter(MaterialObject o, const Ray r_in, HitRecord *rec, Vector3 *attenuation, Ray *scattered) {
	Metal m = o.metal;

	Vector3 reflected = Vector3Reflect(UnitVector(r_in.direction), rec->normal);
	*scattered = ray(rec->p, Vector3Add(
		reflected,
		Vector3Scale(random_in_unit_sphere(), m.roughness)
	));

	if (dot(scattered->direction, rec->normal) > 0) {
		*attenuation = m.albedo;
		return true;
	}

	return false;
}

bool Dielectric_scatter(MaterialObject o, const Ray r_in, HitRecord *rec, Vector3 *attenuation, Ray *scattered) {
	Dielectric d = o.dielectric;

	*attenuation = color(1.0, 1.0, 1.0);
	double refraction_ratio = rec->front_face ? (1.0/d.ior) : d.ior;

	Vector3 unit_direction = UnitVector(r_in.direction);
	double cos_theta = fmin(dot(Vector3Negate(unit_direction), rec->normal), 1.0);
	double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

	bool cannot_refract = refraction_ratio * sin_theta > 1.0;
	Vector3 direction;

	if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double1())
		direction = Vector3Reflect(unit_direction, rec->normal);
	else
		direction = Vector3Refract(unit_direction, rec->normal, refraction_ratio);
	*scattered = ray(rec->p, direction);
	return true;
}

bool Emissive_scatter(MaterialObject o, const Ray r_in, HitRecord *rec, Vector3 *attenuation, Ray *scattered) {
	Emissive e = o.emissive;
	*attenuation = Vector3Scale(e.color, e.brighness);
	return false;
}

Mat MakeLambertian(Vector3 albedo) {
	Mat s;
	s.scatter = Lambertian_scatter;
	s.object.lambertian = (Lambertian){albedo};
	return s;
}

Mat MakeMetal(Vector3 albedo, float roughness) {
	Mat s;
	s.scatter = Metal_scatter;
	s.object.metal = (Metal){albedo, roughness < 1 ? roughness : 1};
	return s;
}

Mat MakeDielectric(double ior) {
	Mat s;
	s.scatter = Dielectric_scatter;
	s.object.dielectric = (Dielectric){ior};
	return s;
}

Mat MakeEmissive(Vector3 color, float brightness) {
	Mat s;
	s.scatter = Emissive_scatter;
	s.object.emissive = (Emissive){color, brightness};
	return s;
}
#endif
