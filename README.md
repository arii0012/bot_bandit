# BOT BANDIT: UNDERWORLD

C++20 + Raylib 5.5 life simulator (borderless fullscreen).

## Build (local)

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
# Windows: build\botbandit.exe
# Linux:   ./build/botbandit
```

Dependencies are downloaded by CMake (Raylib 5.5, nlohmann/json).

## GitHub Actions

Push to `main` → builds **Windows `.exe`** and Linux binary as artifacts.  
Tag `v1.0.0` → creates a Release with both packages.

## Controls

| Key | Action |
|-----|--------|
| 1–9 | Work / Rob / Heist / Travel / Shop / Craft / Daily / Garage / Business |
| I | Inventory |
| P | Phone |
| Tab | Radial menu |
| ~ | Console |
| Esc | Pause |
| In drift: WASD, Space handbrake, Shift nitro, F exit |

## Console (examples)

```
/give_money 1000000
/tp москва
/weather rain
/night
/drift
/custom
/auction
/heal
/fixcar
/race
/home
/studio
/fight
/save
```

## Systems included

Cities, heists, craft, phone, garage, tuning, drift + tires, exhaust particles, engine audio,  
businesses, smart home, music studio, fighting, racing, auction, damage, inventory, cutscenes, save.


## Windows installer

GitHub Actions builds `BotBandit-Setup-1.0.0.exe` (Inno Setup):
- choose install folder
- desktop shortcut (checkbox)
- launch after install (checkbox)
- Start menu entry + uninstall

## Exit game

- **Esc** → Pause → **Выход**
- Character creator → **ВЫХОД** or Esc
- Window close / Alt+F4
