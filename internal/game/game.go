package game

import (
	"bufio"
	"fmt"
	"math/rand"
	"os"
	"strconv"
	"strings"
	"time"

	"botbandit/internal/city"
	"botbandit/internal/player"
)

type Game struct {
	P       *player.Player
	Reader  *bufio.Reader
	Running bool
}

func New(p *player.Player) *Game {
	return &Game{
		P:       p,
		Reader:  bufio.NewReader(os.Stdin),
		Running: true,
	}
}

func (g *Game) Run() {
	printBanner()
	fmt.Println("BOT BANDIT: UNDERWORLD  •  Go Edition")
	fmt.Println("Напиши help чтобы увидеть все команды")

	for g.Running {
		g.printStatus()
		fmt.Print("\n> ")
		input, err := g.Reader.ReadString('\n')
		if err != nil {
			break
		}
		input = strings.TrimSpace(input)
		if input == "" {
			continue
		}

		parts := strings.Fields(input)
		cmd := strings.ToLower(parts[0])
		args := parts[1:]

		switch cmd {
		case "help", "помощь", "h", "?":
			g.cmdHelp()
		case "status", "статус", "me", "я", "info":
			fmt.Println(g.P.Summary())
		case "work", "работа", "w":
			g.cmdWork(args)
		case "rob", "ограбить", "r":
			g.cmdRob()
		case "heist", "ограбление", "налет", "налёт":
			g.cmdHeist()
		case "train", "качалка", "тренировка":
			g.cmdTrain(args)
		case "travel", "ехать", "t":
			g.cmdTravel(args)
		case "cities", "города", "map", "карта":
			g.cmdCities()
		case "shop", "магазин":
			g.cmdShop()
		case "inventory", "инвентарь", "inv", "i":
			g.cmdInventory()
		case "daily", "ежедневка":
			g.cmdDaily()
		case "skills", "навыки":
			g.cmdSkills(args)
		case "casino", "казино":
			g.cmdCasino(args)
		case "gang", "банда":
			g.cmdGang(args)
		case "crypto", "крипта":
			g.cmdCrypto(args)
		case "darknet", "даркнет":
			g.cmdDarknet()
		case "hide", "спрятаться":
			g.cmdHide()
		case "sleep", "спать", "отдых":
			g.cmdSleep()
		case "quest", "квесты", "q":
			g.cmdQuests()
		case "achievements", "достижения", "ach":
			g.cmdAchievements()
		case "craft", "крафт", "создать":
			g.cmdCraft(args)
		case "scavenge", "поиск", "лут":
			g.cmdScavenge()
		case "save", "сохранить":
			if err := g.P.Save(); err != nil {
				fmt.Println("❌ Ошибка сохранения:", err)
			} else {
				fmt.Println("✓ Сохранено")
			}
		case "quit", "exit", "выход":
			g.P.Save()
			fmt.Println("До встречи, бандит. Прогресс сохранён.")
			g.Running = false
		case "clear", "cls":
			fmt.Print("\033[H\033[2J")
		default:
			fmt.Printf("Неизвестная команда «%s». Напиши help\n", cmd)
		}

		// пассивное снижение тепла
		if rand.Float64() < 0.12 {
			g.P.LowerHeat(1)
		}
		g.P.PlayTime += 8
	}
}

func (g *Game) printStatus() {
	fmt.Println("───────────────────────────────────────────────────────────────")
	fmt.Println(g.P.StatusLine())
}

func (g *Game) cmdHelp() {
	fmt.Println(`
╔══════════════════════════════════════════════════════════════════╗
║                 BOT BANDIT — ПОЛНЫЙ СПИСОК КОМАНД                ║
╠══════════════════════════════════════════════════════════════════╣
║  ОСНОВНОЕ                                                        ║
║  help / h              — этот список                             ║
║  status / me / я       — полный статус персонажа                 ║
║  save                  — сохранить игру                          ║
║  quit / exit           — выход (с сохранением)                   ║
║                                                                  ║
║  ЗАРАБОТОК И КРИМИНАЛ                                            ║
║  work [номер]          — легальная работа (work 1)               ║
║  rob                   — быстрое уличное ограбление              ║
║  heist                 — ★ СЕРЬЁЗНОЕ ОГРАБЛЕНИЕ (система)        ║
║  casino [ставка]       — казино                                  ║
║                                                                  ║
║  ПЕРЕМЕЩЕНИЕ И ГОРОДА                                            ║
║  cities / map          — список всех городов                     ║
║  travel <город> [car]  — поехать (travel москва car)             ║
║                                                                  ║
║  ПРОКАЧКА                                                        ║
║  train strength|agility|intelligence                             ║
║  skills [buy <skill>]  — навыки (stealth, driving, shooting...)  ║
║  daily                 — ежедневная награда                      ║
║                                                                  ║
║  ЭКИПИРОВКА И МАГАЗИН                                            ║
║  shop                  — магазин оружия/гаджетов/топлива         ║
║  inventory / inv       — инвентарь                               ║
║                                                                  ║
║  НОВЫЕ СИСТЕМЫ                                                   ║
║  gang [join|leave|info]— банды                                   ║
║  crypto [buy|sell|price]— криптовалюта                           ║
║  darknet               — чёрный рынок (Москва/Питер/Влад)        ║
║  hide                  — снизить тепло и розыск                  ║
║  sleep                 — полностью восстановить энергию          ║
║  quest / q             — задания                                 ║
║  achievements          — достижения                              ║
║                                                                  ║
║  КРАФТ                                                           ║
║  craft                 — система крафта (рецепты)                ║
║  craft <рецепт>        — скрафтить предмет                       ║
║  scavenge / лут        — поискать материалы на улице             ║
╚══════════════════════════════════════════════════════════════════╝`)
}

// ===================== РАБОТА =====================

func (g *Game) cmdWork(args []string) {
	info := g.P.CityInfo()
	idx := 0
	if len(args) > 0 {
		if n, err := strconv.Atoi(args[0]); err == nil {
			idx = n - 1
		}
	}

	if len(args) == 0 {
		fmt.Println("\nДоступные работы в", info.Name+":")
		for i, j := range info.Jobs {
			fmt.Printf("  %d. %-28s ~%d₽  (кд %dс)\n", i+1, j.Name, g.P.Price(j.BasePay), j.Cooldown)
		}
		fmt.Println("\nИспользование: work 1")
		return
	}

	earned, msgs, name, errMsg := g.P.Work(idx)
	if errMsg != "" {
		fmt.Println("❌", errMsg)
		return
	}
	fmt.Printf("✓ %s: +%d₽\n", name, earned)
	for _, m := range msgs {
		fmt.Println(" ", m)
	}
}

// ===================== БЫСТРОЕ ОГРАБЛЕНИЕ =====================

func (g *Game) cmdRob() {
	ok, amount, msgs, errMsg := g.P.Rob()
	if errMsg != "" {
		fmt.Println("❌", errMsg)
		return
	}
	if ok {
		fmt.Printf("💀 Успех! Украдено +%d₽\n", amount)
	} else {
		fmt.Println("Провал. Пришлось бежать.")
	}
	for _, m := range msgs {
		fmt.Println(" ", m)
	}
}

// ===================== СЕРЬЁЗНАЯ СИСТЕМА ОГРАБЛЕНИЙ (HEIST) =====================

func (g *Game) cmdHeist() {
	if g.P.Level < 2 {
		fmt.Println("❌ Для серьёзных дел нужен минимум 2 уровень")
		return
	}
	if g.P.Energy < 35 {
		fmt.Println("❌ Нужно минимум 35 энергии")
		return
	}
	if !g.P.LastHeist.IsZero() && time.Since(g.P.LastHeist) < 35*time.Minute {
		left := 35*time.Minute - time.Since(g.P.LastHeist)
		fmt.Printf("❌ Следующий налёт доступен через %v\n", left.Round(time.Minute))
		return
	}
	if g.P.WantedLevel >= 5 {
		fmt.Println("❌ Розыск максимальный. Сначала спрячься (hide) или купи документы в darknet")
		return
	}

	fmt.Println(`
╔════════════════════════════════════════════════════════════╗
║              ★ СИСТЕМА СЕРЬЁЗНЫХ ОГРАБЛЕНИЙ ★              ║
╠════════════════════════════════════════════════════════════╣
║  Выбери цель:                                              ║
║                                                            ║
║  1. Киоск / Магазин у дома                                 ║
║     Риск: низкий     Награда: 25–70 тыс                   ║
║                                                            ║
║  2. Ювелирка / Ломбард                                     ║
║     Риск: средний    Награда: 90–220 тыс                  ║
║                                                            ║
║  3. Отделение банка                                        ║
║     Риск: высокий    Награда: 200–550 тыс                 ║
║                                                            ║
║  4. Инкассаторская машина                                  ║
║     Риск: очень высокий  Награда: 400–1.2 млн             ║
║                                                            ║
║  5. Частный особняк (только если есть банда)               ║
║     Риск: экстремальный  Награда: 800к–2.5 млн            ║
╚════════════════════════════════════════════════════════════╝`)
	fmt.Print("Выбери цель (1-5): ")
	line, _ := g.Reader.ReadString('\n')
	choice, _ := strconv.Atoi(strings.TrimSpace(line))

	type Target struct {
		Name      string
		Chance    float64
		MinReward int64
		MaxReward int64
		Heat      int
		NeedGang  bool
		MinLevel  int
	}

	targets := map[int]Target{
		1: {"Киоск", 0.78, 25000, 70000, 10, false, 2},
		2: {"Ювелирка", 0.55, 90000, 220000, 22, false, 3},
		3: {"Банк", 0.38, 200000, 550000, 35, false, 5},
		4: {"Инкассаторы", 0.28, 400000, 1200000, 48, false, 7},
		5: {"Особняк", 0.22, 800000, 2500000, 60, true, 8},
	}

	t, ok := targets[choice]
	if !ok {
		fmt.Println("Отмена.")
		return
	}
	if g.P.Level < t.MinLevel {
		fmt.Printf("❌ Для этой цели нужен %d уровень\n", t.MinLevel)
		return
	}
	if t.NeedGang && g.P.Gang == "" {
		fmt.Println("❌ Для особняка нужна банда (gang join ...)")
		return
	}

	// --- Планирование ---
	fmt.Println("\n📋 ПЛАНИРОВАНИЕ НАЛЁТА")
	fmt.Println("1. Идти в одиночку (быстро, меньше шанс)")
	fmt.Println("2. Взять напарника из банды (+шанс, но делишь добычу)")
	fmt.Println("3. Подготовить инструменты (отмычки / глушилка) — стоит 15–40к")
	fmt.Print("Выбор подхода (1-3): ")
	approachLine, _ := g.Reader.ReadString('\n')
	approach, _ := strconv.Atoi(strings.TrimSpace(approachLine))

	chance := t.Chance
	rewardMult := 1.0
	extraCost := int64(0)
	solo := true

	switch approach {
	case 2:
		if g.P.Gang == "" {
			fmt.Println("Нет банды — идёшь один")
		} else {
			solo = false
			chance += 0.12
			rewardMult = 0.65 // делишь
			fmt.Println("✓ Напарник из банды готов")
		}
	case 3:
		cost := int64(15000 + rand.Intn(25000))
		if g.P.Money < cost {
			fmt.Println("Не хватает денег на инструменты — идёшь как есть")
		} else {
			g.P.Money -= cost
			extraCost = cost
			chance += 0.18
			fmt.Printf("✓ Купил инструменты за %d₽\n", cost)
		}
	default:
		fmt.Println("Идёшь в одиночку")
	}

	// навыки
	chance += float64(g.P.Skills["stealth"]) * 0.045
	chance += float64(g.P.Skills["shooting"]) * 0.025
	chance += float64(g.P.Agility) * 0.008
	chance -= float64(g.P.WantedLevel) * 0.07
	chance -= float64(g.P.CityInfo().DangerLevel) * 0.025

	if chance > 0.92 {
		chance = 0.92
	}
	if chance < 0.08 {
		chance = 0.08
	}

	fmt.Printf("\nШанс успеха: %.0f%%\n", chance*100)
	fmt.Print("Начать налёт? (да/нет): ")
	confirm, _ := g.Reader.ReadString('\n')
	confirm = strings.ToLower(strings.TrimSpace(confirm))
	if confirm != "да" && confirm != "y" && confirm != "yes" && confirm != "д" {
		fmt.Println("Отменено.")
		if extraCost > 0 {
			g.P.Money += extraCost // вернём деньги за инструменты
		}
		return
	}

	g.P.Energy -= 30 + rand.Intn(15)
	if g.P.Energy < 0 {
		g.P.Energy = 0
	}
	g.P.LastHeist = time.Now()
	g.P.Stats["heist"]++

	fmt.Println("\n⏳ Идёт налёт...")
	time.Sleep(1800 * time.Millisecond)

	roll := rand.Float64()
	if roll < chance {
		// УСПЕХ
		base := t.MinReward + rand.Int63n(t.MaxReward-t.MinReward+1)
		amount := int64(float64(base) * rewardMult)

		// бонусы
		if g.P.Skills["negotiation"] > 2 && rand.Float64() < 0.3 {
			bonus := int64(float64(amount) * 0.15)
			amount += bonus
			fmt.Printf("💰 Переговорный навык дал +%d₽ сверху\n", bonus)
		}

		g.P.Money += amount
		g.P.Heat += t.Heat
		if g.P.Heat > 100 {
			g.P.Heat = 100
		}

		fmt.Printf("\n💀 УСПЕХ! Цель: %s\n", t.Name)
		fmt.Printf("   Украдено: %s₽\n", formatMoney(amount))

		if !solo {
			fmt.Println("   Напарник забрал свою долю.")
		}

		// шанс повышения розыска
		if rand.Float64() < 0.25+float64(t.Heat)/200 {
			g.P.WantedLevel++
			if g.P.WantedLevel > 5 {
				g.P.WantedLevel = 5
			}
			fmt.Printf("🚨 Розыск повышен до %d★\n", g.P.WantedLevel)
		}

		msgs := g.P.AddExp(60 + int(amount/15000))
		for _, m := range msgs {
			fmt.Println(" ", m)
		}

		// достижение
		if g.P.Stats["heist"] >= 1 {
			g.P.CheckAchievement("first_heist")
		}
		if amount >= 500000 {
			g.P.CheckAchievement("big_score")
		}

	} else {
		// ПРОВАЛ
		g.P.Heat += t.Heat + 18
		if g.P.Heat > 100 {
			g.P.Heat = 100
		}
		g.P.WantedLevel++
		if g.P.WantedLevel > 5 {
			g.P.WantedLevel = 5
		}

		fmt.Println("\n💥 ПРОВАЛ!")
		fmt.Println("   Сигнализация / свидетели / патруль.")
		fmt.Printf("🚨 Розыск теперь %d★ | Тепло: %d\n", g.P.WantedLevel, g.P.Heat)

		// потери
		loss := int64(12000 + g.P.Level*2500) + int64(t.Heat)*800
		if g.P.Money >= loss {
			g.P.Money -= loss
			fmt.Printf("   Потерял %s₽ при побеге\n", formatMoney(loss))
		}

		// шанс ранения
		if rand.Float64() < 0.35 {
			g.P.Energy -= 20
			if g.P.Energy < 0 {
				g.P.Energy = 0
			}
			fmt.Println("   🩹 Получил ранение, энергия снижена")
		}
	}
}

func formatMoney(m int64) string {
	if m >= 1_000_000 {
		return fmt.Sprintf("%.2f млн", float64(m)/1e6)
	}
	if m >= 1000 {
		return fmt.Sprintf("%d тыс", m/1000)
	}
	return fmt.Sprintf("%d", m)
}

// ===================== ОСТАЛЬНЫЕ КОМАНДЫ =====================

func (g *Game) cmdTrain(args []string) {
	if len(args) == 0 {
		fmt.Println("Использование: train strength | agility | intelligence")
		return
	}
	stat := strings.ToLower(args[0])
	switch stat {
	case "str", "сила":
		stat = "strength"
	case "agi", "ловкость":
		stat = "agility"
	case "int", "интеллект":
		stat = "intelligence"
	}
	ok, msg := g.P.Train(stat)
	if ok {
		fmt.Println("✓", msg)
	} else {
		fmt.Println("❌", msg)
	}
}

func (g *Game) cmdTravel(args []string) {
	if len(args) == 0 {
		g.cmdCities()
		fmt.Println("\nИспользование: travel <город> [car]")
		return
	}
	to := strings.ToLower(args[0])
	useCar := len(args) > 1 && (args[1] == "car" || args[1] == "машина" || args[1] == "авто")

	found := ""
	for id, c := range city.Cities {
		if id == to || strings.Contains(strings.ToLower(c.Name), to) {
			found = id
			break
		}
	}
	if found == "" {
		fmt.Println("❌ Город не найден. Смотри cities")
		return
	}

	cost, sec, msgs, errMsg := g.P.Travel(found, useCar)
	if errMsg != "" {
		fmt.Println("❌", errMsg)
		return
	}
	fmt.Printf("🚗 В пути в %s (%d сек, -%d₽)...\n", city.Cities[found].Name, sec, cost)
	wait := sec
	if wait > 3 {
		wait = 3
	}
	time.Sleep(time.Duration(wait) * time.Second)
	fmt.Printf("✓ Прибыл в %s\n", city.Cities[found].Name)
	for _, m := range msgs {
		fmt.Println(" ", m)
	}
}

func (g *Game) cmdCities() {
	fmt.Println("\n📍 ГОРОДА:")
	for _, id := range city.GetCityList() {
		c := city.Cities[id]
		mark := "  "
		if id == g.P.City {
			mark = "▶ "
		}
		fmt.Printf("%s%-18s %s  [опасность %d/5]\n", mark, c.Name, c.Desc, c.DangerLevel)
	}
}

func (g *Game) cmdShop() {
	fmt.Println(`
🛒 МАГАЗИН
1. Нож — 8 000₽
2. Пистолет — 45 000₽
3. SMG — 185 000₽
4. Смартфон — 25 000₽
5. Энергетик (+40 энергии) — 1 200₽
6. Канистра топлива (+30) — 3 500₽
7. Лицензия на оружие — 150 000₽
0. Выход`)
	fmt.Print("Выбор: ")
	line, _ := g.Reader.ReadString('\n')
	switch strings.TrimSpace(line) {
	case "1":
		g.buyItem("Нож", 8000, "weapon")
	case "2":
		g.buyItem("Пистолет", 45000, "weapon")
	case "3":
		g.buyItem("SMG", 185000, "weapon")
	case "4":
		g.buyItem("Смартфон", 25000, "gadget")
	case "5":
		if g.P.Money >= 1200 {
			g.P.Money -= 1200
			g.P.Energy += 40
			if g.P.Energy > 100 {
				g.P.Energy = 100
			}
			fmt.Println("✓ Энергия:", g.P.Energy)
		} else {
			fmt.Println("❌ Не хватает")
		}
	case "6":
		if g.P.ActiveCar == "" {
			fmt.Println("❌ Нет активной машины")
			return
		}
		if g.P.Money >= 3500 {
			g.P.Money -= 3500
			g.P.Fuel += 30
			if g.P.Fuel > g.P.MaxFuel && g.P.MaxFuel > 0 {
				g.P.Fuel = g.P.MaxFuel
			}
			fmt.Printf("✓ Топливо %d/%d\n", g.P.Fuel, g.P.MaxFuel)
		} else {
			fmt.Println("❌ Не хватает")
		}
	case "7":
		if g.P.HasLicense {
			fmt.Println("Уже есть лицензия")
			return
		}
		if g.P.Money >= 150000 {
			g.P.Money -= 150000
			g.P.HasLicense = true
			fmt.Println("✓ Лицензия получена")
		} else {
			fmt.Println("❌ Нужно 150 000₽")
		}
	}
}

func (g *Game) buyItem(name string, price int64, typ string) {
	if g.P.Money < price {
		fmt.Println("❌ Не хватает денег")
		return
	}
	g.P.Money -= price
	switch typ {
	case "weapon":
		g.P.Inventory.Weapons = append(g.P.Inventory.Weapons, name)
	case "gadget":
		g.P.Inventory.Gadgets = append(g.P.Inventory.Gadgets, name)
	}
	fmt.Println("✓ Купил", name)
}

func (g *Game) cmdInventory() {
	fmt.Println("\n📦 ИНВЕНТАРЬ")
	fmt.Printf("  Одежда:  %v\n", g.P.Inventory.Clothes)
	fmt.Printf("  Машины:  %v\n", g.P.Inventory.Cars)
	fmt.Printf("  Дома:    %v\n", g.P.Inventory.Houses)
	fmt.Printf("  Гаджеты: %v\n", g.P.Inventory.Gadgets)
	fmt.Printf("  Оружие:  %v\n", g.P.Inventory.Weapons)
	fmt.Printf("  Активная машина: %s (топливо %d/%d)\n", g.P.ActiveCar, g.P.Fuel, g.P.MaxFuel)
}

func (g *Game) cmdDaily() {
	reward, msgs, errMsg := g.P.ClaimDaily()
	if errMsg != "" {
		fmt.Println("❌", errMsg)
		return
	}
	fmt.Printf("🎁 Ежедневная награда: +%d₽\n", reward)
	for _, m := range msgs {
		fmt.Println(" ", m)
	}
}

func (g *Game) cmdSkills(args []string) {
	fmt.Println("\n🎯 НАВЫКИ")
	for k, v := range g.P.Skills {
		fmt.Printf("  %-14s %d\n", k, v)
	}
	fmt.Printf("\nСвободных очков: %d\n", g.P.SkillPoints)
	if len(args) >= 2 && args[0] == "buy" {
		ok, msg := g.P.BuySkill(args[1])
		if ok {
			fmt.Println("✓", msg)
		} else {
			fmt.Println("❌", msg)
		}
	} else {
		fmt.Println("Прокачать: skills buy stealth")
	}
}

func (g *Game) cmdCasino(args []string) {
	if !g.P.CityInfo().HasCasino {
		fmt.Println("❌ В этом городе нет казино")
		return
	}
	bet := int64(5000)
	if len(args) > 0 {
		if n, err := strconv.ParseInt(args[0], 10, 64); err == nil && n >= 1000 {
			bet = n
		}
	}
	if g.P.Money < bet {
		fmt.Println("❌ Не хватает денег")
		return
	}
	g.P.Money -= bet
	g.P.Stats["casino"]++

	roll := rand.Intn(100)
	var mult float64
	var title string
	switch {
	case roll < 48:
		mult, title = 0, "Проигрыш"
	case roll < 78:
		mult, title = 1.6, "x1.6"
	case roll < 92:
		mult, title = 2.5, "x2.5"
	case roll < 98:
		mult, title = 5, "x5"
	default:
		mult, title = 12, "★ ДЖЕКПОТ x12"
	}
	win := int64(float64(bet) * mult)
	g.P.Money += win
	if win > bet {
		fmt.Printf("🎰 %s! +%d₽\n", title, win-bet)
	} else {
		fmt.Printf("🎰 %s. -%d₽\n", title, bet)
	}
}

func (g *Game) cmdGang(args []string) {
	if len(args) == 0 {
		if g.P.Gang == "" {
			fmt.Println("Ты свободен. Банды: Южные, Урал, Столичные, Дальний, Чёрные")
			fmt.Println("gang join <название>")
		} else {
			fmt.Printf("🏴 Банда «%s» | Ранг %d\n", g.P.Gang, g.P.GangRank)
		}
		return
	}
	switch args[0] {
	case "join":
		if g.P.Gang != "" {
			fmt.Println("❌ Сначала leave")
			return
		}
		if len(args) < 2 {
			return
		}
		name := strings.Title(strings.ToLower(args[1]))
		g.P.Gang = name
		g.P.GangRank = 1
		fmt.Printf("✓ Вступил в «%s»\n", name)
	case "leave":
		g.P.Gang = ""
		g.P.GangRank = 0
		fmt.Println("Покинул банду")
	case "info":
		fmt.Printf("Банда: %s | Ранг: %d\n", g.P.Gang, g.P.GangRank)
	}
}

func (g *Game) cmdCrypto(args []string) {
	prices := map[string]float64{
		"BTC":    94500 + rand.Float64()*4000,
		"ETH":    3550 + rand.Float64()*180,
		"BANDIT": 0.18 + rand.Float64()*0.12,
	}
	if len(args) == 0 {
		fmt.Println("\n💰 КРИПТОПОРТФЕЛЬ")
		for k, v := range g.P.Crypto {
			fmt.Printf("  %-8s %.6f   (~$%.2f)\n", k, v, prices[k])
		}
		fmt.Println("\ncrypto price | crypto buy BTC 0.01 | crypto sell ETH 0.5")
		return
	}
	switch args[0] {
	case "price":
		for k, v := range prices {
			fmt.Printf("  %s = $%.2f\n", k, v)
		}
	case "buy":
		if len(args) < 3 {
			return
		}
		coin := strings.ToUpper(args[1])
		amount, _ := strconv.ParseFloat(args[2], 64)
		price, ok := prices[coin]
		if !ok || amount <= 0 {
			return
		}
		cost := int64(amount * price * 95)
		if g.P.Money < cost {
			fmt.Println("❌ Не хватает")
			return
		}
		g.P.Money -= cost
		g.P.Crypto[coin] += amount
		fmt.Printf("✓ Купил %.6f %s за %d₽\n", amount, coin, cost)
	case "sell":
		if len(args) < 3 {
			return
		}
		coin := strings.ToUpper(args[1])
		amount, _ := strconv.ParseFloat(args[2], 64)
		if g.P.Crypto[coin] < amount {
			fmt.Println("❌ Недостаточно монет")
			return
		}
		gain := int64(amount * prices[coin] * 92)
		g.P.Crypto[coin] -= amount
		g.P.Money += gain
		fmt.Printf("✓ Продал %.6f %s за %d₽\n", amount, coin, gain)
	}
}

func (g *Game) cmdDarknet() {
	id := g.P.City
	if id != "москва" && id != "питер" && id != "владивосток" {
		fmt.Println("❌ Даркнет только в Москве, Питере и Владивостоке")
		return
	}
	fmt.Println(`
🕸️  ДАРКНЕТ
1. Фальшивые документы (розыск -2, тепло -30) — 85 000₽
2. Сброс тепла (-25) — 40 000₽
3. Инфа о крупном деле — 25 000₽
0. Выход`)
	fmt.Print("> ")
	line, _ := g.Reader.ReadString('\n')
	switch strings.TrimSpace(line) {
	case "1":
		if g.P.Money >= 85000 {
			g.P.Money -= 85000
			g.P.WantedLevel -= 2
			if g.P.WantedLevel < 0 {
				g.P.WantedLevel = 0
			}
			g.P.LowerHeat(30)
			fmt.Println("✓ Документы получены")
		} else {
			fmt.Println("❌ Не хватает")
		}
	case "2":
		if g.P.Money >= 40000 {
			g.P.Money -= 40000
			g.P.LowerHeat(25)
			fmt.Println("✓ Тепло снижено")
		} else {
			fmt.Println("❌ Не хватает")
		}
	case "3":
		if g.P.Money >= 25000 {
			g.P.Money -= 25000
			fmt.Println("📡 Инфа: через пару дней инкассаторы повезут крупняк по югу...")
		}
	}
}

func (g *Game) cmdHide() {
	if g.P.Energy < 8 {
		fmt.Println("❌ Нет сил")
		return
	}
	g.P.Energy -= 8
	g.P.LowerHeat(14 + rand.Intn(9))
	fmt.Printf("✓ Спрятался. Тепло сейчас: %d\n", g.P.Heat)
}

func (g *Game) cmdSleep() {
	g.P.Energy = 100
	g.P.LowerHeat(6)
	fmt.Println("✓ Выспался. Энергия 100")
}

func (g *Game) cmdQuests() {
	fmt.Println(`
📜 ЗАДАНИЯ
• Сделай 3 успешных heist
• Посети 6 разных городов
• Заработай 2 000 000₽
• Вступи в банду и дослужись до ранга 3
• Купи любую крипту
(прогресс отслеживается автоматически через achievements)`)
}

func (g *Game) cmdAchievements() {
	fmt.Println("\n🏆 ДОСТИЖЕНИЯ")
	list := []struct{ ID, Title string }{
		{"first_rob", "Первое уличное ограбление"},
		{"first_heist", "Первый серьёзный налёт"},
		{"big_score", "Украл больше 500к за раз"},
		{"rich", "Состояние 1 000 000₽+"},
		{"traveler", "10+ поездок"},
	}
	for _, a := range list {
		st := "❌"
		if g.P.Achievements[a.ID] {
			st = "✅"
		}
		fmt.Printf("  %s  %s\n", st, a.Title)
	}
}


// ===================== СИСТЕМА КРАФТА =====================

type Recipe struct {
	ID          string
	Name        string
	Desc        string
	Materials   map[string]int
	Result      string
	ResultType  string // weapon, gadget, item, consumable
	MinLevel    int
	SkillReq    string // optional skill
	SkillLevel  int
}

var Recipes = []Recipe{
	{
		ID: "lockpick", Name: "Отмычка", Desc: "Повышает шанс в heist и rob",
		Materials: map[string]int{"metal": 2, "scrap": 3},
		Result: "Отмычка", ResultType: "item", MinLevel: 1,
	},
	{
		ID: "medkit", Name: "Аптечка", Desc: "Восстанавливает 50 энергии",
		Materials: map[string]int{"fabric": 2, "chemicals": 2},
		Result: "Аптечка", ResultType: "consumable", MinLevel: 1,
	},
	{
		ID: "silencer", Name: "Глушитель", Desc: "+12% к успеху heist",
		Materials: map[string]int{"metal": 4, "electronics": 2, "scrap": 2},
		Result: "Глушитель", ResultType: "item", MinLevel: 3, SkillReq: "shooting", SkillLevel: 1,
	},
	{
		ID: "hacktool", Name: "Взломщик", Desc: "Нужен для продвинутых дел и darknet-бонусов",
		Materials: map[string]int{"electronics": 5, "metal": 2},
		Result: "Взломщик", ResultType: "gadget", MinLevel: 4, SkillReq: "hacking", SkillLevel: 1,
	},
	{
		ID: "armor", Name: "Самодельный бронежилет", Desc: "Снижает потери при провале heist",
		Materials: map[string]int{"fabric": 5, "metal": 3, "scrap": 4},
		Result: "Бронежилет", ResultType: "item", MinLevel: 5,
	},
	{
		ID: "emp", Name: "ЭМИ-заряд", Desc: "Сильно повышает шанс на банк/инкассаторов",
		Materials: map[string]int{"electronics": 6, "chemicals": 3, "metal": 2},
		Result: "ЭМИ-заряд", ResultType: "item", MinLevel: 6, SkillReq: "hacking", SkillLevel: 2,
	},
	{
		ID: "knife_up", Name: "Заточенный нож", Desc: "Улучшенное холодное оружие",
		Materials: map[string]int{"metal": 3, "scrap": 2},
		Result: "Заточенный нож", ResultType: "weapon", MinLevel: 2,
	},
	{
		ID: "energy_drink", Name: "Крутой энергетик", Desc: "Полностью восстанавливает энергию",
		Materials: map[string]int{"chemicals": 3, "scrap": 1},
		Result: "Крутой энергетик", ResultType: "consumable", MinLevel: 1,
	},
}

func (g *Game) cmdCraft(args []string) {
	if len(args) == 0 {
		fmt.Println("\n🔧 СИСТЕМА КРАФТА")
		fmt.Println("Материалы у тебя:")
		for mat, qty := range g.P.Materials {
			fmt.Printf("  %-14s %d\n", mat, qty)
		}
		fmt.Println("\nДоступные рецепты:")
		for _, r := range Recipes {
			mats := ""
			for m, q := range r.Materials {
				mats += fmt.Sprintf("%s×%d ", m, q)
			}
			lock := ""
			if g.P.Level < r.MinLevel {
				lock = fmt.Sprintf(" [нужен ур.%d]", r.MinLevel)
			}
			if r.SkillReq != "" && g.P.Skills[r.SkillReq] < r.SkillLevel {
				lock += fmt.Sprintf(" [нужен %s %d]", r.SkillReq, r.SkillLevel)
			}
			fmt.Printf("  %-12s → %-22s (%s)%s\n", r.ID, r.Name, mats, lock)
			fmt.Printf("               %s\n", r.Desc)
		}
		fmt.Println("\nИспользование: craft <id>   например: craft medkit")
		fmt.Println("Добыча материалов: scavenge")
		return
	}

	id := strings.ToLower(args[0])
	var recipe *Recipe
	for i := range Recipes {
		if Recipes[i].ID == id {
			recipe = &Recipes[i]
			break
		}
	}
	if recipe == nil {
		fmt.Println("❌ Нет такого рецепта. Напиши craft без аргументов")
		return
	}
	if g.P.Level < recipe.MinLevel {
		fmt.Printf("❌ Нужен уровень %d\n", recipe.MinLevel)
		return
	}
	if recipe.SkillReq != "" && g.P.Skills[recipe.SkillReq] < recipe.SkillLevel {
		fmt.Printf("❌ Нужен навык %s уровня %d\n", recipe.SkillReq, recipe.SkillLevel)
		return
	}

	// check materials
	for mat, need := range recipe.Materials {
		if g.P.Materials[mat] < need {
			fmt.Printf("❌ Не хватает материала «%s» (есть %d, нужно %d)\n", mat, g.P.Materials[mat], need)
			return
		}
	}

	// consume
	for mat, need := range recipe.Materials {
		g.P.Materials[mat] -= need
	}

	// give result
	switch recipe.ResultType {
	case "weapon":
		g.P.Inventory.Weapons = append(g.P.Inventory.Weapons, recipe.Result)
	case "gadget":
		g.P.Inventory.Gadgets = append(g.P.Inventory.Gadgets, recipe.Result)
	case "item", "consumable":
		g.P.Inventory.Items = append(g.P.Inventory.Items, recipe.Result)
	}

	fmt.Printf("✓ Скрафтил: %s\n", recipe.Result)
	g.P.AddExp(20)
}

func (g *Game) cmdScavenge() {
	if g.P.Energy < 12 {
		fmt.Println("❌ Нужно минимум 12 энергии")
		return
	}
	g.P.Energy -= 10 + rand.Intn(5)

	// chance based on city danger + intelligence
	rolls := 2 + g.P.Intelligence/3
	if rolls > 5 {
		rolls = 5
	}

	gains := map[string]int{}
	mats := []string{"scrap", "electronics", "chemicals", "fabric", "metal"}
	for i := 0; i < rolls; i++ {
		mat := mats[rand.Intn(len(mats))]
		qty := 1 + rand.Intn(3)
		if rand.Float64() < 0.15 {
			qty += 2
		}
		g.P.Materials[mat] += qty
		gains[mat] += qty
	}

	fmt.Println("🔍 Поискал по району...")
	if len(gains) == 0 {
		fmt.Println("Ничего полезного не нашёл.")
		return
	}
	for mat, qty := range gains {
		fmt.Printf("  +%d %s\n", qty, mat)
	}
	g.P.AddExp(8)
}


func printBanner() {
	fmt.Println(`
██████╗  ██████╗ ████████╗    ██████╗  █████╗ ███╗   ██╗██████╗ ██╗████████╗
██╔══██╗██╔═══██╗╚══██╔══╝    ██╔══██╗██╔══██╗████╗  ██║██╔══██╗██║╚══██╔══╝
██████╔╝██║   ██║   ██║       ██████╔╝███████║██╔██╗ ██║██║  ██║██║   ██║   
██╔══██╗██║   ██║   ██║       ██╔══██╗██╔══██║██║╚██╗██║██║  ██║██║   ██║   
██████╔╝╚██████╔╝   ██║       ██████╔╝██║  ██║██║ ╚████║██████╔╝██║   ██║   
╚═════╝  ╚═════╝    ╚═╝       ╚═════╝ ╚═╝  ╚═╝╚═╝  ╚═══╝╚═════╝ ╚═╝   ╚═╝   
                    U N D E R W O R L D   •   G O   E D I T I O N`)
}
