# BOT BANDIT: UNDERWORLD

2.5D life simulator on **C++20 + Raylib 5.5**.

Borderless fullscreen, rounded UI, cities, cars, heists, crafting, phone, cutscenes, dev console.

## Build

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
./build/botbandit
```

Raylib and nlohmann/json are downloaded automatically by CMake.

## Controls

| Key | Action |
|-----|--------|
| Tab | Radial menu |
| ~ | Dev console |
| Esc | Pause menu |
| 1-9 | Quick actions |
| LMB | Click UI |
| Enter | Confirm |

## Dev Console (~)

```
/give_money 100000
/spawn_car lambo
/tp москва
/wanted 0
/godmode
/unlock_all
/char_editor
/exit
```

## GitHub Actions

Push → Windows + Linux artifacts. Tag `v1.0.0` → Release with binaries.
