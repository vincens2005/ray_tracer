#ifndef WORLD
#define WORLD
#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <string.h>
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

typedef struct {
	void* left; // these are void pointers because the Hittable type has not yet been defined. I know this is cursed but YOLO or whatever
	void* right;
	Aabb box;
} BVHNode;

typedef union {
	Sphere sphere;
	Aabb aabb;
	BVHNode bvh_node;
} HittableObject;

typedef struct {
	HittableObject object;
	bool (*hit)(HittableObject o, const Ray r, double t_min, double t_max, HitRecord *rec);
	bool (*bounding_box)(HittableObject o, Aabb* output_box);
	void (*print)(HittableObject o, char* tab);
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
	// find the nearest t that lies in the acceptable range.
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

void Sphere_print(HittableObject o, char* tab) {
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

void Aabb_print(HittableObject o, char* tab) {
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

Aabb surrounding_box(Aabb *box0, Aabb *box1) {
	Vector3 small = {
		fmin(box0->minimum.x, box1->minimum.x),
		fmin(box0->minimum.y, box1->minimum.y),
		fmin(box0->minimum.z, box1->minimum.z)
	};

	Vector3 big = {
		fmax(box0->maximum.x, box1->maximum.x),
		fmax(box0->maximum.y, box1->maximum.y),
		fmax(box0->maximum.z, box1->maximum.z)
	};

	return (Aabb){small, big};
}

Hittable MakeAabb(Vector3 minimum, Vector3 maximum) {
	Hittable a;
	a.hit = Aabb_hit;
	a.print = Aabb_print;
	a.object.aabb = (Aabb){minimum, maximum};
	return a;
}

bool BVHNode_boundingbox(HittableObject o, Aabb* output_box) {
	*output_box = o.bvh_node.box;
	return true;
}

void BVHNode_print(HittableObject o, char* tab) {
	char* new_tab = malloc((strlen(tab) + 1) * sizeof(char));
	sprintf(new_tab, "%s\t", tab);

	printf("BVH Node:\r\n%s\t", tab);
	(*(Hittable*)(o.bvh_node.left)).print((*(Hittable*)(o.bvh_node.left)).object, new_tab);
	printf("%s\t", tab);
	(*(Hittable*)(o.bvh_node.right)).print((*(Hittable*)(o.bvh_node.right)).object, new_tab);
}

bool BVHNode_hit(HittableObject o, const Ray r, double t_min, double t_max, HitRecord *rec) {
	HittableObject box;
	box.aabb = o.bvh_node.box;
	if(!Aabb_hit(box, r, t_min, t_max, rec))
		return false;

	bool hit_left = (*(Hittable*)(o.bvh_node.left)).hit((*(Hittable*)(o.bvh_node.left)).object, r, t_min, t_max, rec);
	bool hit_right = (*(Hittable*)(o.bvh_node.right)).hit((*(Hittable*)(o.bvh_node.right)).object, r, t_min, hit_left ? rec->t : t_max, rec);

	return hit_left || hit_right;
}

int box_compare(Hittable* a, Hittable *b, int axis) {
	Aabb box_a;
	Aabb box_b;

	if (!a->bounding_box(a->object, &box_a) || !b->bounding_box(b->object, &box_b)) {
		printf("SOMETHING HAS GONE TERRIBLY WRONG COMPARING BVH NODES\r\n");
		exit(1);
	}

	double a_min[3] = vec2arr(box_a.minimum);
	double b_min[3] = vec2arr(box_b.minimum);

	return a_min[axis] < b_min[axis] ? 1 : -1;
}

int box_x_compare(const void* a, const void* b) {
	return box_compare((Hittable*)a, (Hittable*)b, 0);
}

int box_y_compare(const void* a, const void* b) {
	return box_compare((Hittable*)a, (Hittable*)b, 1);
}

int box_z_compare(const void* a, const void* b) {
	return box_compare((Hittable*)a, (Hittable*)b, 2);
}


Hittable* MakeBVHNode(Hittable* objects, size_t start, size_t end) {
	Hittable *left, *right;
	size_t object_span = end - start;

	int axis = rand() % 3;

	int (*comparator)(const void* a, const void* b) = axis == 0 ? box_x_compare : axis == 1 ? box_y_compare : box_z_compare;

	if (object_span == 1) {
		left = right = objects + start;
	}
	else if (object_span == 2) {
		if (comparator(objects + start, objects + start + 1)) {
			left = &objects[start];
			right = &objects[start + 1];
		}
		else {
			left = &objects[start + 1];
			right = &objects[start];
		}
	}
	else {
		qsort(objects + start, object_span, sizeof(Hittable), comparator);

		size_t mid = start + object_span / 2;
		left = MakeBVHNode(objects, start, mid);
		right = MakeBVHNode(objects, mid, end);
	}

	Aabb box_left, box_right;

	if (!left->bounding_box(left->object, &box_left) || !right->bounding_box(right->object, &box_right)) {
		printf("SOMEHTHING HAS GONE TERRIBLY WRONG WHILE BUILDING THE BVH\r\n");
		exit(1);
	}

	Aabb box = surrounding_box(&box_left, &box_right);

	Hittable* b = malloc(sizeof(Hittable));
	b->object.bvh_node = (BVHNode){left, right, box};
	b->hit = BVHNode_hit;
	b->print = BVHNode_print;
	b->bounding_box = BVHNode_boundingbox;

	return b; // this can cause a memory leak if we ever need to update the BVH, so BEWARE
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
