#include "Fighting.hpp"
#include "Character/Character.hpp"
#include <cstdlib>
#include <string>

std::string FightingClub::Fight(Character& p, int stake) {
  if (p.Energy() < 25) return "Нужно 25 энергии";
  if (p.Money() < stake) return "Нужна ставка " + std::to_string(stake);
  p.AddMoney(-stake);
  p.SetEnergy(p.Energy() - 20);
  int power = p.Strength() * 10 + p.Agility() * 6 + p.Level() * 3 + (rand() % 40);
  int enemy = 30 + p.Level() * 8 + (rand() % 50);
  if (power >= enemy) {
    int win = stake * 2 + p.Strength() * 200;
    p.AddMoney(win);
    p.AddExp(40);
    return "Победа в клетке! +" + std::to_string(win);
  }
  p.AddHeat(5);
  return "Проигрыш боя. Ставка сгорела.";
}
