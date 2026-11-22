#include "Auction.hpp"
#include "Character/Character.hpp"
#include <cstdlib>

std::vector<AuctionLot> CarAuction::CurrentLots() {
  return {
    {"rust_vaz", "Ржавый ВАЗ (контейнер)", 45000, 120000, 1.05f, "На запчасти или реставрация"},
    {"rust_bmw", "Битый BMW e39", 280000, 900000, 1.55f, "Мотор живой, кузов мёртвый"},
    {"rust_jdm", "JDM-сюрприз", 500000, 1500000, 2.1f, "Что внутри — узнаешь после оплаты"},
    {"rust_truck", "Старый гелик-каркас", 800000, 4000000, 1.5f, "Для тех, кто верит"},
  };
}

std::string CarAuction::BuyLot(Character& p, int index) {
  auto lots = CurrentLots();
  if (index < 0 || index >= (int)lots.size()) return "Нет лота";
  auto& L = lots[index];
  if (p.Money() < L.price) return "Нужно " + std::to_string(L.price);
  p.AddMoney(-L.price);
  p.Inv().items.push_back("project:" + L.id);
  p.Inv().materials["repair_" + L.id] = L.repairCost;
  return "Куплен лот: " + L.name + ". Ремонт ~" + std::to_string(L.repairCost);
}
