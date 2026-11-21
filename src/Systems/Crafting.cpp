#include "Crafting.hpp"
#include "Character/Character.hpp"
#include <cstdlib>

const std::vector<CraftRecipe>& CraftingSystem::Recipes() {
  static std::vector<CraftRecipe> R = {
    {"lockpick", "Отмычка", "+шанс rob/heist",
     {{"metal", 2}, {"scrap", 3}}, "Отмычка", "item", 1, "", 0, 0.95f},
    {"medkit", "Аптечка", "+50 энергии",
     {{"fabric", 2}, {"chemicals", 2}}, "Аптечка", "consumable", 1, "", 0, 1.0f},
    {"energy", "Крутой энергетик", "Полная энергия",
     {{"chemicals", 3}, {"scrap", 1}}, "Крутой энергетик", "consumable", 1, "", 0, 1.0f},
    {"knife_up", "Заточенный нож", "Оружие ближнего боя",
     {{"metal", 3}, {"scrap", 2}}, "Заточенный нож", "weapon", 2, "", 0, 0.9f},
    {"silencer", "Глушитель", "+12% heist",
     {{"metal", 4}, {"electronics", 2}, {"scrap", 2}}, "Глушитель", "item", 3, "shooting", 1, 0.85f},
    {"hacktool", "Взломщик", "Для darknet/сейфов",
     {{"electronics", 5}, {"metal", 2}}, "Взломщик", "gadget", 4, "hacking", 1, 0.8f},
    {"armor", "Бронежилет", "Меньше потерь при провале",
     {{"fabric", 5}, {"metal", 3}, {"scrap", 4}}, "Бронежилет", "item", 5, "", 0, 0.88f},
    {"emp", "ЭМИ-заряд", "Бонус к банку/инкассаторам",
     {{"electronics", 6}, {"chemicals", 3}, {"metal", 2}}, "ЭМИ-заряд", "item", 6, "hacking", 2, 0.75f},
  };
  return R;
}

bool CraftingSystem::CanCraft(const Character& p, const CraftRecipe& r, std::string& reason) {
  if (p.Level() < r.minLevel) {
    reason = "Нужен ур." + std::to_string(r.minLevel);
    return false;
  }
  if (!r.skillReq.empty() && p.Skill(r.skillReq) < r.skillLevel) {
    reason = "Нужен " + r.skillReq + " " + std::to_string(r.skillLevel);
    return false;
  }
  for (auto& [mat, need] : r.cost) {
    int have = 0;
    auto it = p.Inv().materials.find(mat);
    if (it != p.Inv().materials.end()) have = it->second;
    if (have < need) {
      reason = "Мало " + mat + " (" + std::to_string(have) + "/" + std::to_string(need) + ")";
      return false;
    }
  }
  reason.clear();
  return true;
}

bool CraftingSystem::Craft(Character& p, const std::string& recipeId, std::string& message) {
  const CraftRecipe* rec = nullptr;
  for (auto& r : Recipes()) if (r.id == recipeId) { rec = &r; break; }
  if (!rec) { message = "Нет рецепта"; return false; }

  std::string reason;
  if (!CanCraft(p, *rec, reason)) { message = reason; return false; }

  // consume full cost first
  for (auto& [mat, need] : rec->cost) p.Inv().materials[mat] -= need;

  float roll = (rand() % 1000) / 1000.f;
  float chance = rec->successChance + p.Skill("hacking") * 0.02f;
  if (chance > 0.98f) chance = 0.98f;

  if (roll > chance) {
    // fail: refund half materials (rounded down)
    for (auto& [mat, need] : rec->cost)
      p.Inv().materials[mat] += need / 2;
    message = "Провал крафта: " + rec->name + " (часть материалов потеряна)";
    return false;
  }

  if (rec->resultType == "weapon") p.Inv().weapons.push_back(rec->resultItem);
  else if (rec->resultType == "gadget") p.Inv().gadgets.push_back(rec->resultItem);
  else p.Inv().items.push_back(rec->resultItem);

  p.AddExp(18 + rec->minLevel * 4);
  message = "Скрафтил: " + rec->name;
  return true;
}

std::string CraftingSystem::FormatRecipe(const CraftRecipe& r) {
  std::string s = r.name + " [";
  bool first = true;
  for (auto& [m, q] : r.cost) {
    if (!first) s += " ";
    s += m + "x" + std::to_string(q);
    first = false;
  }
  s += "]";
  return s;
}
