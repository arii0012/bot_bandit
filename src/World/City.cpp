#include "City.hpp"

static std::vector<City> gCities = {
  {"lipeck", "Lipetsk", "Promyshlennyy start", 0.72f, 0, 1, false, true, false, false, false, false, false,
   {{"Zavod", 3800, 22}, {"Gruzchik", 2400, 16}, {"Kurer", 1900, 11}}, 0.35f, 0.45f},
  {"voronezh", "Voronezh", "Studencheskiy, kazino", 0.88f, 45, 2, true, true, false, false, false, false, false,
   {{"Ofitsiant", 2600, 14}, {"Repetitor", 3400, 18}}, 0.32f, 0.58f},
  {"rostov", "Rostov", "Yug, tachki", 0.95f, 75, 3, true, true, true, false, true, false, false,
   {{"Torgovets", 3200, 16}, {"Taksist", 3700, 20}}, 0.25f, 0.70f},
  {"krasnodar", "Krasnodar", "Kuban", 0.80f, 65, 2, false, true, true, false, false, false, false,
   {{"Sbor", 2800, 13}, {"Fura", 4200, 24}}, 0.22f, 0.82f},
  {"sochi", "Sochi", "Kurort, drift-serpantin", 1.40f, 130, 2, true, true, true, true, true, false, false,
   {{"Animator", 4100, 17}, {"Barmen", 4500, 19}}, 0.18f, 0.88f},
  {"kazan", "Kazan", "IT, avtodrom", 1.05f, 90, 2, true, true, true, true, true, true, false,
   {{"Frilans", 6000, 25}, {"Menedzher", 3800, 18}}, 0.42f, 0.38f},
  {"nizhniy", "Nizhniy", "Volga", 0.90f, 55, 2, false, true, false, false, false, false, false,
   {{"Zavod", 3600, 20}, {"Port", 3100, 15}}, 0.48f, 0.32f},
  {"moskva", "Moskva", "Stolitsa, birzha", 1.85f, 200, 5, true, true, true, true, false, true, true,
   {{"VIP-kurer", 7500, 18}, {"Ohrana", 9000, 22}}, 0.55f, 0.28f},
  {"piter", "Sankt-Peterburg", "Kultura, aukt siony", 1.55f, 150, 4, true, true, true, true, false, true, false,
   {{"Ekskursovod", 4200, 16}, {"IT", 7000, 23}}, 0.58f, 0.18f},
  {"ekaterinburg", "Ekaterinburg", "Ural", 0.98f, 80, 4, true, true, true, false, false, false, false,
   {{"Shahtyor", 4500, 22}, {"Dalnoboy", 4800, 20}}, 0.62f, 0.42f},
  {"novosibirsk", "Novosibirsk", "Sibir", 0.92f, 70, 3, false, true, false, false, false, false, false,
   {{"Laborant", 3800, 18}, {"Logist", 4100, 17}}, 0.72f, 0.48f},
  {"irkutsk", "Irkutsk", "Baykal", 0.85f, 50, 2, false, true, false, false, false, false, false,
   {{"Gid", 3900, 16}, {"Lesorub", 3600, 19}}, 0.82f, 0.52f},
  {"vladivostok", "Vladivostok", "Port, Aziya", 1.15f, 110, 4, true, true, true, true, false, false, false,
   {{"Port", 4200, 16}, {"Tamozhnya", 5500, 20}}, 0.95f, 0.55f},
};

const std::vector<City>& AllCities() { return gCities; }

const City* FindCity(const std::string& id) {
  for (auto& c : gCities)
    if (c.id == id) return &c;
  // Cyrillic legacy ids map
  if (id == "липецк" || id == "lipeck") return FindCity("lipeck");
  if (id == "москва" || id == "moskva") return FindCity("moskva");
  if (id == "питер" || id == "piter") return FindCity("piter");
  if (id == "сочи" || id == "sochi") return FindCity("sochi");
  if (id == "казань" || id == "kazan") return FindCity("kazan");
  if (id == "воронеж" || id == "voronezh") return FindCity("voronezh");
  return nullptr;
}
