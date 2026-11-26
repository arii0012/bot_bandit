#include "City.hpp"

static std::vector<City> gCities = {
  {"lipeck", "Липецк", "Промышленный старт", 0.72f, 0, 1, false, true, false, false, false, false, false,
   {{"Завод", 3800, 22}, {"Грузчик", 2400, 16}, {"Курьер", 1900, 11}}, 0.35f, 0.45f},
  {"voronezh", "Воронеж", "Студенческий район, казино", 0.88f, 45, 2, true, true, false, false, false, false, false,
   {{"Официант", 2600, 14}, {"Репетитор", 3400, 18}}, 0.32f, 0.58f},
  {"rostov", "Ростов-на-Дону", "Юг, автомобили", 0.95f, 75, 3, true, true, true, false, true, false, false,
   {{"Торговец", 3200, 16}, {"Таксист", 3700, 20}}, 0.25f, 0.70f},
  {"krasnodar", "Краснодар", "Кубань", 0.80f, 65, 2, false, true, true, false, false, false, false,
   {{"Сборщик", 2800, 13}, {"Дальнобойщик", 4200, 24}}, 0.22f, 0.82f},
  {"sochi", "Сочи", "Курорт, горные дороги", 1.40f, 130, 2, true, true, true, true, true, false, false,
   {{"Аниматор", 4100, 17}, {"Бармен", 4500, 19}}, 0.18f, 0.88f},
  {"kazan", "Казань", "IT и автодром", 1.05f, 90, 2, true, true, true, true, true, true, false,
   {{"Фрилансер", 6000, 25}, {"Менеджер", 3800, 18}}, 0.42f, 0.38f},
  {"nizhniy", "Нижний Новгород", "Волга", 0.90f, 55, 2, false, true, false, false, false, false, false,
   {{"Завод", 3600, 20}, {"Порт", 3100, 15}}, 0.48f, 0.32f},
  {"moskva", "Москва", "Столица, биржа", 1.85f, 200, 5, true, true, true, true, false, true, true,
   {{"VIP-курьер", 7500, 18}, {"Охранник", 9000, 22}}, 0.55f, 0.28f},
  {"piter", "Санкт-Петербург", "Культура, аукционы", 1.55f, 150, 4, true, true, true, true, false, true, false,
   {{"Экскурсовод", 4200, 16}, {"IT", 7000, 23}}, 0.58f, 0.18f},
  {"ekaterinburg", "Екатеринбург", "Урал", 0.98f, 80, 4, true, true, true, false, false, false, false,
   {{"Шахтёр", 4500, 22}, {"Дальнобойщик", 4800, 20}}, 0.62f, 0.42f},
  {"novosibirsk", "Новосибирск", "Сибирь", 0.92f, 70, 3, false, true, false, false, false, false, false,
   {{"Лаборант", 3800, 18}, {"Логист", 4100, 17}}, 0.72f, 0.48f},
  {"irkutsk", "Иркутск", "Байкал", 0.85f, 50, 2, false, true, false, false, false, false, false,
   {{"Гид", 3900, 16}, {"Лесоруб", 3600, 19}}, 0.82f, 0.52f},
  {"vladivostok", "Владивосток", "Порт, Азия", 1.15f, 110, 4, true, true, true, true, false, false, false,
   {{"Порт", 4200, 16}, {"Таможенник", 5500, 20}}, 0.95f, 0.55f},
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
