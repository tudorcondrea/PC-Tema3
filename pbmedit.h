#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

typedef struct{
	int x;
	int y;
} coordinate;

typedef struct{
	coordinate min;
	coordinate max;
} corner;

typedef struct{
	int r;
	int g;
	int b;
} triplet;

triplet **load(char filename[], corner *c_limits,
			   int *intensity, int *magic_word);
corner resize(int x1, int y1, int x2, int y2);
triplet **transpose(triplet **img, corner *cor);
void swap_rows(triplet **img, corner cor);
int validate_corners(corner c, corner c_limits);
triplet **crop(triplet **img, corner *c_crop, corner c_restrict);
void grayscale(triplet **img, corner c_restrict);
void sepia(triplet **img, corner c_restrict);
void save(char filename[], int magic_word, corner c_limits,
		  int intensity, triplet **img, int ascii);
double minimum(double a, double b);
