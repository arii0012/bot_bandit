# BOT BANDIT: UNDERWORLD (C++20 / Raylib 5.5)

Borderless fullscreen life-sim. Cities, heists, craft, phone, garage, businesses, weather, cutscenes.

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/botbandit
```

## Controls

| Key | Action |
|-----|--------|
| 1 | Work |
| 2 | Rob |
| 3 | Heist |
| 4 | Travel |
| 5 | Shop |
| 6 | Craft / scavenge |
| 7 | Daily |
| 8 | Garage / cars / tuning |
| 9 | Business |
| P | Smartphone |
| Tab | Radial menu |
| ~ | Dev console |
| Esc | Pause |

## Console

```
/give_money 100000
/tp москва
/wanted 0
/weather rain|clear|neon|fog
/spawn_car lambo
/garage
/biz
/cutscene debut|keys|heist_win|arrest|travel
/godmode
/unlock_all
/save
/exit
```

## Systems

- 13 cities, day/night, weather (rain traction)
- Work, rob, heist (with cutscenes)
- Crafting (8 recipes)
- Smartphone: Bank, Crypto, DarkNet, Social, Taxi
- Garage + tuning (engine, turbo, neon, paint)
- Business buy/upgrade/collect
- Cutscenes: debut, keys, heist, arrest
- Save JSON, GitHub Actions CI

## More systems

- `/race` — street race / pink slip
- `/home` — smart home (needs mansion)
- `/studio` — music tracks & royalties
- `/autopilot` — Tesla/Mercedes travel
- `/fight` `/pet` `/smuggle`
