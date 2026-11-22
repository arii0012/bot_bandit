#include "DJ.hpp"
#include "Character/Character.hpp"
#include <cstdlib>

std::string MusicStudio::ProduceTrack(Character& p) {
  if (p.Energy() < 15) return "Нужно 15 энергии";
  if (p.Money() < 8000) return "Студия: 8000 за сессию";
  p.AddMoney(-8000);
  p.SetEnergy(p.Energy() - 12);
  int quality = p.Intelligence() * 5 + p.Skill("negotiation") * 3 + (rand() % 40);
  p.Inv().materials["tracks"] += 1;
  if (quality > 50) p.Inv().materials["hits"] += 1;
  p.AddExp(25);
  return quality > 50 ? "Трек-ХИТ записан!" : "Трек записан (обычный)";
}

std::string MusicStudio::CollectRoyalties(Character& p) {
  int tracks = p.Inv().materials["tracks"];
  int hits = p.Inv().materials["hits"];
  if (tracks <= 0 && hits <= 0) return "Нет треков на радио";
  int pay = tracks * 1500 + hits * 8000;
  p.AddMoney(pay);
  return "Роялти с радио: +" + std::to_string(pay);
}
