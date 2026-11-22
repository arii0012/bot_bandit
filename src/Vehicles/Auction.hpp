#pragma once
#include <string>
#include <vector>

class Character;

struct AuctionLot {
  std::string id;
  std::string name;
  int price;
  int repairCost;
  float potentialSpeed;
  std::string desc;
};

class CarAuction {
public:
  static std::vector<AuctionLot> CurrentLots();
  static std::string BuyLot(Character& p, int index);
};
