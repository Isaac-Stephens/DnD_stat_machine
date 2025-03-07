#include "character.h"
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include "json.hpp"  // JSON library for parsing

using namespace std;
using json = nlohmann::json;

std::map<std::string, ClassInfo> classData;     // Stores all class info from JSON
std::map<std::string, RaceInfo> raceData;       // Stores all race info from JSON

void loadClassData(){}  // TODO Load Classes from JSON

void loadRaceData(){}   // TODO Load Race from JSON

Character::Character() { // Default constructor (set for testing purposes)
    name = "test_name";
    race = "test_race";
    classLevels = {{"Fighter", 3}, {"Barbarian", 2}};
    totalLevel = 5;
    prof_bonus = 3;
    
    strengh = 2;
    dexterity = 16;
    constitution = 10;
    intelligence = 10;
    wisdom = 10;
    charisma = 10;

    HP_Max = 15;
    HP = 15;
    tmp_HP_Max = 0;
    tmp_HP = 0;

    AC = 8;
    Bonus_AC = 0;

    abilities = {};

}

Character::Character(string name, string race) {}   // TODO Constructor

// void Character::addClass(string className, int levels = 1) {} // TODO Add classes from JSON

/* Display Functions */

string Character::getName() const {return name;}
string Character::getRace() const {return race;}
int Character::getLevel(string className) const {} // TODO Return given class level

int Character::getTotalLevel() const {return totalLevel;}
int Character::getMaxHitPoints() const {return HP_Max;}
int Character::getHitPoints() const {return HP;}
int Character::getTmpMaxHP() const {return tmp_HP_Max;}
int Character::getTmpHP() const {return tmp_HP;}
int Character::getAC() const {return AC;}
int Character::getBonusAC() const {return Bonus_AC;}
int Character::getStr() const {return strengh;}
int Character::getDex() const {return dexterity;}
int Character::getCon() const {return constitution;}
int Character::getInt() const {return intelligence;}
int Character::getWis() const {return wisdom;}
int Character::getChr() const {return charisma;}

int Character::getAbilityMod(int ability) {return (ability/2)-5;}

/* Update Functions */

void Character::damageHP(int dmg) { // TODO Make tpm hp take dmg first
    if (HP > 0) { // prevents negative HP
        if (dmg > HP)
            HP = 0;
        else
            HP -= dmg;
    }
}

void Character::healHP(int heal) {
    if (HP + heal > HP_Max) // prevents > max hp
        HP = HP_Max;
    else
        HP += heal;
}

void Character::addTmpHP(int tmp) {

}

void Character::addBonusAC(int bonus) {

}

