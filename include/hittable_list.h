#ifndef HITTABLELIST
#define HITTABLELIST
#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include "utils.h"
#include "world.h"

// HittableList type and functions
typedef struct {
	Hittable* objects;
	int len;
	Mat* materials;
	int mat_len;
	bool changed;
	Cam camera;
} HittableList;

void HittableList_clear(HittableList* list) {
	free(list->objects);
	free(list->materials);
	list->materials = (Mat*)malloc(sizeof(Mat));
	list->objects = (Hittable*)malloc(sizeof(Hittable));
	list->len = 0;
	list->mat_len = 0;
}

void HittableList_add(HittableList* list, Hittable obj) {
	list->len++;
	list->objects = (Hittable*)realloc(list->objects, sizeof(Hittable) * list->len);
	list->objects[list->len - 1] = obj;
}

int HittableList_addMat(HittableList* list, Mat mat) {
	list->mat_len++;
	list->materials = (Mat*)realloc(list->materials, sizeof(Mat) * list->mat_len);
	list->materials[list->mat_len - 1] = mat;
	return list->mat_len - 1;
}

HittableList MakeHittableList() {
	return (HittableList){
		 (Hittable*)malloc(sizeof(Hittable)),
		 0,
		 (Mat*)malloc(sizeof(Mat)),
		 0,
		 false
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

	printf("%smaterials:\r\n", tabulation);
	for (int i = 0; i < l->mat_len; i++) {
		printf("%s\tmaterial address: %p\r\n", tabulation, &(l->materials[i]));
	}
}
#endif
