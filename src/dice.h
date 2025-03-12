// dice.h
#ifndef DICE_H
#define DICE_H

#include <random>

// Each die function default rolls 1 die
// Overloaded verions take an integer n and roll n dice

int d_custom(int faces);
int d_custom(int faces, int n);

int d_100();
int d_100(int);

int d_20();
int d_20(int);

int d_12();
int d_12(int);

int d_10();
int d_10(int);

int d_8();
int d_8(int);

int d_6();
int d_6(int);

int d_4();
int d_4(int);



#endif