#pragma once
#include <string>
class Character;
class MusicStudio {
public:
  static std::string ProduceTrack(Character& p);
  static std::string CollectRoyalties(Character& p);
};
