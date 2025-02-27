// dice.cpp
#include "dice.h"
#include <random>

std::random_device rd;
std::mt19937 gen(rd());



// Rolls a die with a specified number of sides
int rollDie(int sides) {
    std::uniform_int_distribution<int> dist(1, sides);
    return dist(gen);
}

// Implementations for each die
int d_100() { return rollDie(100); }
int d_100(int n) {
    int roll = 0;
    for (int i=0; i<n; i++) {
        roll += rollDie(100);
    }
    return roll;
}

int d_20() { return rollDie(20); }
int d_20(int n) {
    int roll = 0;
    for (int i=0; i<n; i++) {
        roll += rollDie(20);
    }
    return roll;
}

int d_12() { return rollDie(12); }
int d_12(int n) {
    int roll = 0;
    for (int i=0; i<n; i++) {
        roll += rollDie(12);
    }
    return roll;
}

int d_10() { return rollDie(10); }
int d_10(int n) {
    int roll = 0;
    for (int i=0; i<n; i++) {
        roll += rollDie(10);
    }
    return roll;
}

int d_8() { return rollDie(8); }
int d_8(int n) {
    int roll = 0;
    for (int i=0; i<n; i++) {
        roll += rollDie(8);
    }
    return roll;
}

int d_6() { return rollDie(6); }
int d_6(int n) {
    int roll = 0;
    for (int i=0; i<n; i++) {
        roll += rollDie(6);
    }
    return roll;
}

int d_4() { return rollDie(4); }
int d_4(int n) {
    int roll = 0;
    for (int i=0; i<n; i++) {
        roll += rollDie(4);
    }
    return roll;
}