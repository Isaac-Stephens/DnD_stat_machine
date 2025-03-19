#ifndef CHARACTER_H
#define CHARACTER_H

#include "dice.h"
#include "json.hpp"
#include <map>
#include <string>
#include <vector>

using namespace std;
using json = nlohmann::json;

struct RaceInfo {
  map<string, int> abilityBonuses;
  int speed;
  string size;
  vector<string> traits;
};

struct ClassInfo {
  int hit_die;
  vector<string> proficiencies;
  map<int, vector<string>>
      features; // int = class lvl, vector = features per level
};

class Character {
private:
  string name;

  string race;

  map<string, int> classLevels; // e.g., {"Fighter" : 3, "Wizard" : 2}

  int totalLevel;

  int prof_bonus;

  map<string, int> abilities;

  int HP_Max, HP, tmp_HP_Max, tmp_HP;

  int AC, Bonus_AC;

  // If true => add prof_bonus to saving throws
  map<string, bool> saves = {{"strength", false},     {"dexterity", false},
                             {"constitution", false}, {"intelligence", false},
                             {"wisdom", false},       {"charisma", false}};
  /* Proficiencies */
  map<string, bool> proficiencies = {
      {"acrobatics", false},      // dex
      {"animal_handling", false}, // wis
      {"arcana", false},          // int
      {"athletics", false},       // str
      {"deception", false},       // chr
      {"history", false},         // int
      {"insight", false},         // wis
      {"intimidation", false},    // chr
      {"investigation", false},   // int
      {"medicine", false},        // wis
      {"nature", false},          // int
      {"perception", false},      // wis
      {"performance", false},     // chr
      {"persuasion", false},      // chr
      {"religion", false},        // int
      {"sleight_of_hand", false}, // dex
      {"stealth", false},         // dex
      {"survival", false}         // wis
  };

  map<string, string> skillToAbility = {{"acrobatics", "dexterity"},
                                        {"animal_handling", "wisdom"},
                                        {"arcana", "intelligence"},
                                        {"athletics", "strength"},
                                        {"deception", "charisma"},
                                        {"history", "intelligence"},
                                        {"insight", "wisdom"},
                                        {"intimidation", "charisma"},
                                        {"investigation", "intelligence"},
                                        {"medicine", "wisdom"},
                                        {"nature", "intelligence"},
                                        {"perception", "wisdom"},
                                        {"performance", "charisma"},
                                        {"persuasion", "charisma"},
                                        {"religion", "intelligence"},
                                        {"sleight_of_hand", "dexterity"},
                                        {"stealth", "dexterity"},
                                        {"survival", "wisdom"}};

  vector<string>
      known_skills; // may be redundant, consider converting ability values to
                    // save and prof maps straight from JSON

  //        json classData;
public:
  Character(); // Default Constructor

  Character(string name, string race); // Constructor

  //        void addClass(string className, int levels = 1);

  /* Display Functions */
  string getName() const;                  // returns name
  string getRace() const;                  // returns race
  map<string, int> getClassLevels() const; // returns classLevels
  int getLevel(string className) const;    // returns level of individual class
                                        // from classLevels from associated name
  int getTotalLevel() const;   // returns totalLevel
  int getMaxHitPoints() const; // returns HP_Max
  int getHitPoints() const;    // returns HP
  int getTmpHP() const;        // returns tmp_HP
  int getAC() const;           // returns AC
  int getBonusAC() const;      // returns Bonus_AC

  map<string, bool> getSavingThrows() const;  // returns saves
  bool *getSave(string save);                 // returns boolean value of save
  map<string, bool> getProficiencies() const; // returns proficiencies
  bool *getProf(string prof);                 // returns boolean value of prof

  map<string, string> getSkillToAbility() const;

  int getAbility(string ability) const;
  int getStr() const;
  int getDex() const;
  int getCon() const;
  int getInt() const;
  int getWis() const;
  int getChr() const;
  int getProfBonus() const;

  int getAbilityMod(int);

  /* Update Functions */
  void damageHP(int);
  void healHP(int);
  void addTmpHP(int);
  void addBonusAC(int);

  void toggleSavingThrow(string);
  void toggleProficiency(string);
};

#endif