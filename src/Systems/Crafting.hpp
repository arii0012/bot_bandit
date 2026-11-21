#pragma once
#include <string>
#include <vector>
#include <map>

struct CraftRecipe {
  std::string id;
  std::string name;
  std::string desc;
  std::map<std::string, int> cost; // material -> amount
  std::string resultItem;
  std::string resultType; // item, weapon, gadget, consumable
  int minLevel = 1;
  std::string skillReq;   // empty = none
  int skillLevel = 0;
  float successChance = 1.0f; // <1 can fail and lose some mats
};

class Character;

class CraftingSystem {
public:
  static const std::vector<CraftRecipe>& Recipes();
  static bool CanCraft(const Character& p, const CraftRecipe& r, std::string& reason);
  // returns true on success; on fail may consume partial mats
  static bool Craft(Character& p, const std::string& recipeId, std::string& message);
  static std::string FormatRecipe(const CraftRecipe& r);
};
