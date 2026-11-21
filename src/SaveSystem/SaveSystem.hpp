#pragma once
#include <string>

class Character;
class World;

class SaveSystem {
public:
  bool Exists() const;
  bool Save(const Character& p, const World& w);
  bool Load(Character& p, World& w);
  std::string Path() const { return "data/save.json"; }
};
