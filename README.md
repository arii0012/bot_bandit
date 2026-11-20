# BOT BANDIT: UNDERWORLD (Go Edition)

Полностью переписанная версия «Бот Бандит» на **Go**.

Криминальный симулятор жизни по 13 городам России с командной системой.

## Возможности

- 13 городов с уникальными работами и опасностью
- Работа, уличные ограбления (`rob`), серьёзные налёты (`heist`)
- Система крафта (`craft` + `scavenge`)
- Навыки: stealth, driving, shooting, hacking, negotiation
- Банды, крипта, даркнет
- Казино, магазин, инвентарь
- Розыск (Wanted ★), тепло, энергия
- Достижения и ежедневные награды
- Сохранения в JSON

## Быстрый старт

```bash
go run .
```

Или:

```bash
go build -o bandit .
./bandit
```

## Основные команды

```
help                  — список всех команд
status / me           — статус персонажа
work 1                — работать
rob                   — быстрое ограбление
heist                 — серьёзное ограбление (с планированием)
craft                 — система крафта
craft medkit          — скрафтить аптечку
scavenge              — поискать материалы
travel москва car     — поехать на машине
train strength        — качалка
skills buy stealth    — прокачать навык
casino 10000
gang join Южные
crypto buy BTC 0.01
darknet
shop
inventory
daily
hide / sleep
save
quit
```

## Крафт

Материалы: `scrap`, `electronics`, `chemicals`, `fabric`, `metal`

Добываются командой `scavenge`.

Примеры рецептов:
- `craft lockpick` — отмычка
- `craft medkit` — аптечка
- `craft silencer` — глушитель
- `craft hacktool` — взломщик
- `craft armor` — бронежилет
- `craft emp` — ЭМИ-заряд

## Структура

```
├── main.go
├── go.mod
├── internal/
│   ├── city/cities.go
│   ├── player/player.go
│   └── game/game.go
└── data/save.json   (создаётся автоматически)
```

## Лицензия

Делай что хочешь. Это фан-проект.

## GitHub Actions

При каждом push в `main` автоматически:

- Проверяется код (`go vet`)
- Собирается под Linux / Windows / macOS
- Бинарники появляются во вкладке **Actions → Artifacts**

Чтобы выпустить релиз с готовыми файлами:

```bash
git tag v1.0.0
git push origin v1.0.0
```

GitHub сам создаст Release и прикрепит бинарники.
