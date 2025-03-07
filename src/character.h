#ifndef CHARACTER_H
#define CHARACTER_H

#include "dice.h"
#include "json.hpp"
#include <string>
#include <vector>
#include <map>

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
    map<int, vector<string>> features; // int = class lvl, vector = features per level
};

class Character {
    private:
        string name;
        
        string race;

        map<string, int> classLevels; // e.g., {"Fighter" : 3, "Wizard" : 2}

        int totalLevel;
        
        int prof_bonus;
        
        int strengh, dexterity, constitution, intelligence, wisdom, charisma;
        
        int HP_Max, HP, tmp_HP_Max, tmp_HP;
        
        int AC, Bonus_AC;
        
        // If true => add prof_bonus to saving throws
        bool str_save=false, con_save=false, int_save=false, wis_save=false, chr_save=false;
        
        /* Proficiencies */
        bool acrobatics=false, animal_handling=false, arcana=false, deception=false, history=false,
            insight=false, intimidation=false, investigation=false, medicine=false, nature=false, 
            perception=false, performance=false, persuasion=false, religion=false, sleight_of_hand=false, 
            stealth=false, survival=false;
        
        vector<string> abilities;
        
//        json classData;
    public:
        Character(); // Default Constructor
    
        Character(string name, string race); // Constructor
        
//        void addClass(string className, int levels = 1);
        
        /* Display Functions */
        string getName() const;
        string getRace() const;
        int getLevel(string className) const;
        int getTotalLevel() const;
        int getMaxHitPoints() const;
        int getHitPoints() const;
        int getTmpMaxHP() const;
        int getTmpHP() const;
        int getAC() const;
        int getBonusAC() const;
        
        int getStr() const;
        int getDex() const;
        int getCon() const;
        int getInt() const;
        int getWis() const;
        int getChr() const;
        int getAbilityMod(int);

        /* Update Functions */
        void damageHP(int);
        void healHP(int);
        void addTmpHP(int);
        void addBonusAC(int);
};

#endif