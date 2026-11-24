#pragma once
#include <string>
#include <vector>

struct ShopItem {
  const char* id;
  const char* name;
  int price;
  int tier;
  const char* kind; // clothes cars houses weapons
};

inline const std::vector<ShopItem>& CatalogClothes() {
  static const std::vector<ShopItem> v = {
    {"clothes_shirt", "Belaya rubashka", 3500, 1, "clothes"},
    {"clothes_hoodie", "Chernaya tolstovka", 5000, 1, "clothes"},
    {"clothes_hoodie2", "Sinyaya hudi", 5500, 1, "clothes"},
    {"clothes_sport", "Sportivka", 6000, 1, "clothes"},
    {"clothes_bandana", "Krasnaya bandana", 2500, 1, "clothes"},
    {"clothes_glasses", "Temnye ochki", 7000, 1, "clothes"},
    {"clothes_jacket", "Kozhanaya kurtka", 25000, 2, "clothes"},
    {"clothes_leather", "Kozhanka premium", 45000, 2, "clothes"},
    {"clothes_suit", "Delovoy kostyum", 55000, 2, "clothes"},
    {"clothes_chain", "Zolotaya tsep", 120000, 3, "clothes"},
  };
  return v;
}

inline const std::vector<ShopItem>& CatalogCars() {
  static const std::vector<ShopItem> v = {
    {"car_prior", "Lada Priora", 450000, 1, "cars"},
    {"car_granta", "Lada Granta", 520000, 1, "cars"},
    {"car_vesta", "Lada Vesta", 980000, 1, "cars"},
    {"car_solaris", "Hyundai Solaris", 1200000, 2, "cars"},
    {"car_camry", "Toyota Camry", 2800000, 2, "cars"},
    {"car_bmw3", "BMW 3 Series", 4500000, 3, "cars"},
    {"car_mers", "Mercedes E-Class", 5200000, 3, "cars"},
    {"car_gtr", "Nissan GT-R", 9000000, 3, "cars"},
    {"car_lambo", "Lamborghini", 25000000, 4, "cars"},
  };
  return v;
}

inline const std::vector<ShopItem>& CatalogHouses() {
  static const std::vector<ShopItem> v = {
    {"house_room", "Koyka v hostele", 80000, 1, "houses"},
    {"house_apartment", "Odushka", 450000, 1, "houses"},
    {"house_center", "Kv visokogo classa", 2500000, 2, "houses"},
    {"house_mansion", "Osobnyak", 15000000, 3, "houses"},
    {"house_live0", "Panelka", 900000, 1, "houses"},
    {"house_live1", "Townhouse", 3500000, 2, "houses"},
    {"house_extra0", "Loft", 5000000, 2, "houses"},
    {"house_extra1", "Penthouse", 22000000, 3, "houses"},
  };
  return v;
}

inline const std::vector<ShopItem>& CatalogWeapons() {
  static const std::vector<ShopItem> v = {
    {"w_knife", "Nozh", 5000, 1, "weapons"},
    {"w_bat", "Bitа", 8000, 1, "weapons"},
    {"w_pistol", "Pistolet", 45000, 2, "weapons"},
    {"w_smg", "SMG", 120000, 3, "weapons"},
    {"w_rifle", "Avtomat", 250000, 3, "weapons"},
  };
  return v;
}

inline const char* CityList[] = {
  "lipeck", "voronezh", "rostov", "krasnodar", "sochi", "kazan",
  "nizhniy", "moskva", "piter", "ekaterinburg", "novosibirsk", "irkutsk", "vladivostok"
};
inline constexpr int CityCount = 13;
