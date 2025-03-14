#include "character.h"
#include "json.hpp" // JSON library for parsing
#include <fstream>
#include <iostream>
#include <map>
#include <string>
#include <vector>

using namespace std;
using json = nlohmann::json;

std::map<std::string, ClassInfo> classData; // Stores all class info from JSON
std::map<std::string, RaceInfo> raceData;   // Stores all race info from JSON

void loadClassData() {} // TODO Load Classes from JSON

void loadRaceData() {} // TODO Load Race from JSON

Character::Character() { // Default constructor (set for testing purposes)
  name = "test_name";
  race = "test_race";
  classLevels = {{"Paladin", 12}, {"Warlock", 4}};
  totalLevel = 16;
  prof_bonus = 5;

  abilities["strength"] = 20;
  abilities["dexterity"] = 13;
  abilities["constitution"] = 12;
  abilities["intelligence"] = 9;
  abilities["wisdom"] = 17;
  abilities["charisma"] = 18;

  HP_Max = 15;
  HP = 15;
  tmp_HP = 0;

  AC = 8;
  Bonus_AC = 0;
}

Character::Character(string name, string race) {} // TODO Constructor

// void Character::addClass(string className, int levels = 1) {} // TODO Add
// classes from JSON

/* Display Functions */

string Character::getName() const { return name; }
string Character::getRace() const { return race; }
map<string, int> Character::getClassLevels() const { return classLevels; }
int Character::getLevel(string className) const {
  return classLevels.at(className);
}
int Character::getTotalLevel() const {
  return totalLevel;
} // FIXME set from classLevels
int Character::getMaxHitPoints() const { return HP_Max; }
int Character::getHitPoints() const { return HP; }
int Character::getTmpHP() const { return tmp_HP; }
int Character::getAC() const { return AC; }
int Character::getBonusAC() const { return Bonus_AC; }

map<string, bool> Character::getSavingThrows() const { return saves; }
bool *Character::getSave(string save) { return &saves[save]; }
map<string, bool> Character::getProficiencies() const { return proficiencies; }
bool *Character::getProf(string prof) { return &proficiencies[prof]; }

map<string, string> Character::getSkillToAbility() const {
  return skillToAbility;
};

int Character::getAbility(string ability) const {
  auto it = abilities.find(ability);
  return (it != abilities.end()) ? it->second : 0; // Default to 0 if missing
}
// ------------------------------------------------------------------------------------------------
// (Redundant? Idk yet, might be usefull but honestly all of these can be done
// w/ getAbility() lol)
int Character::getStr() const {
  auto it = abilities.find("strength");
  return (it != abilities.end()) ? it->second : 0; // Default to 0 if missing
}
int Character::getDex() const {
  auto it = abilities.find("dexterity");
  return (it != abilities.end()) ? it->second : 0; // Default to 0 if missing
}
int Character::getCon() const {
  auto it = abilities.find("constitution");
  return (it != abilities.end()) ? it->second : 0; // Default to 0 if missing
}
int Character::getInt() const {
  auto it = abilities.find("intelligence");
  return (it != abilities.end()) ? it->second : 0; // Default to 0 if missing
}
int Character::getWis() const {
  auto it = abilities.find("wisdom");
  return (it != abilities.end()) ? it->second : 0; // Default to 0 if missing
}
int Character::getChr() const {
  auto it = abilities.find("charisma");
  return (it != abilities.end()) ? it->second : 0; // Default to 0 if missing
}
// ------------------------------------------------------------------------------------------------

int Character::getProfBonus() const { return prof_bonus; }

int Character::getAbilityMod(int ability) { return ability / 2 - 5; }

/* Update Functions */

void Character::damageHP(int dmg) { // TODO Make tpm hp take dmg first
  if (HP > 0) {                     // prevents negative HP
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

void Character::addTmpHP(int tmp) { tmp_HP += tmp; }

void Character::addBonusAC(int bonus) { Bonus_AC += bonus; }

void Character::toggleSavingThrow(string save) { saves[save] = !saves[save]; }

void Character::toggleProficiency(string prof) {
  proficiencies[prof] = !proficiencies[prof];
}
