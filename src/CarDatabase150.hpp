#pragma once
#include <string>
#include <vector>
#include <array>
#include "raylib.h"

struct CarDef {
  const char* id;
  const char* brand;
  const char* model;
  int price;
  float speed;
  float handling;
  unsigned char r, g, b;
};

class CarDatabase150 {
public:
  static const std::vector<CarDef>& All() {
    static std::vector<CarDef> db;
    if (!db.empty()) return db;
    struct B { const char* brand; std::array<const char*, 10> models; int base; float spd; };
    B brands[] = {
      {"Mercedes-Benz", {"A-Class","C-Class","E-Class","S-Class","G-Class","GLE","GLS","AMG GT","CLS","Maybach"}, 3500000, 1.35f},
      {"BMW", {"1 Series","3 Series","5 Series","7 Series","X5","X6","M3","M5","iX","Z4"}, 3200000, 1.4f},
      {"Audi", {"A3","A4","A6","A8","Q5","Q7","RS6","R8","e-tron","TT"}, 3000000, 1.38f},
      {"Porsche", {"911","Cayenne","Macan","Panamera","Taycan","718","GT3","Turbo S","Carrera","Spyder"}, 9000000, 1.7f},
      {"Lamborghini", {"Huracan","Aventador","Urus","Revuelto","Gallardo","Murcielago","Sián","STO","Performante","Espada"}, 25000000, 1.95f},
      {"Ferrari", {"488","F8","Roma","SF90","812","Portofino","296","Purosangue","LaFerrari","Testarossa"}, 28000000, 1.98f},
      {"McLaren", {"720S","765LT","Artura","GT","P1","Senna","600LT","650S","570S","Elva"}, 22000000, 1.92f},
      {"Rolls-Royce", {"Phantom","Ghost","Wraith","Cullinan","Dawn","Spectre","Silver","Boat Tail","Cord","Park Ward"}, 35000000, 1.2f},
      {"Toyota", {"Camry","Corolla","RAV4","Land Cruiser","Supra","Prius","Highlander","Yaris","C-HR","Avalon"}, 1800000, 1.15f},
      {"Lexus", {"IS","ES","LS","RX","LX","NX","RC","LC","UX","GX"}, 4500000, 1.3f},
      {"Nissan", {"GT-R","350Z","370Z","Qashqai","X-Trail","Patrol","Leaf","Silvia","Juke","Maxima"}, 2500000, 1.45f},
      {"Tesla", {"Model 3","Model Y","Model S","Model X","Cybertruck","Roadster","Semi","Plaid","Long Range","Performance"}, 5000000, 1.75f},
      {"Zeekr", {"001","009","X","007","001 FR","ES","EX","GT","SUV","Sedan"}, 4000000, 1.5f},
      {"Lada", {"Vesta","Granta","Niva","Largus","XRAY","Priora","Kalina","4x4","Vesta SW","Iskra"}, 900000, 0.95f},
      {"UAZ", {"Hunter","Patriot","Bukhanka","Pickup","Profi","Classic","Expedition","Cargo","Hunter Turbo","Soft"}, 1200000, 0.9f},
    };
    int idn = 0;
    for (auto& b : brands) {
      for (int i = 0; i < 10; ++i) {
        CarDef d;
        static char ids[150][32];
        snprintf(ids[idn], 32, "%s_%d", b.brand, i);
        d.id = ids[idn];
        d.brand = b.brand;
        d.model = b.models[i];
        d.price = b.base + i * (b.base / 12);
        d.speed = b.spd + i * 0.02f;
        d.handling = 0.9f + (i % 5) * 0.05f;
        d.r = (unsigned char)(40 + (idn * 37) % 180);
        d.g = (unsigned char)(40 + (idn * 53) % 180);
        d.b = (unsigned char)(40 + (idn * 19) % 180);
        db.push_back(d);
        idn++;
      }
    }
    return db;
  }

  static const CarDef* Find(const std::string& id) {
    for (auto& c : All()) if (id == c.id) return &c;
    return nullptr;
  }

  static int Count() { return (int)All().size(); }
};

struct HouseDef {
  const char* id;
  const char* name;
  int price;
  int type; // 0 house 1 club 2 loft 3 mansion
};

inline const HouseDef* HouseCatalog() {
  static const HouseDef H[20] = {
    {"h_panel", "Panelka", 2500000, 0},
    {"h_cottage", "Cottage", 8000000, 0},
    {"h_town", "Townhouse", 12000000, 0},
    {"h_loft", "Loft", 15000000, 2},
    {"h_club_neon", "Neon Club", 25000000, 1},
    {"h_club_vip", "VIP Club", 40000000, 1},
    {"h_mansion", "Mansion", 80000000, 3},
    {"h_penthouse", "Penthouse", 60000000, 3},
    {"h_garage_house", "House+Garage", 18000000, 0},
    {"h_safehouse", "Safehouse", 5000000, 0},
    {"h_studio", "Music Studio Loft", 22000000, 2},
    {"h_bunker", "Bunker", 30000000, 0},
    {"h_villa", "Villa", 95000000, 3},
    {"h_club_jazz", "Jazz Club", 28000000, 1},
    {"h_bar", "Bar Property", 14000000, 1},
    {"h_warehouse", "Warehouse", 10000000, 0},
    {"h_farm", "Suburban Farm", 7000000, 0},
    {"h_tower", "Tower Suite", 70000000, 3},
    {"h_club_underground", "Underground Club", 35000000, 1},
    {"h_smart", "Smart Home", 45000000, 0},
  };
  return H;
}
