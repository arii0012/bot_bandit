package player

import (
	"encoding/json"
	"fmt"
	"math"
	"math/rand"
	"os"
	"path/filepath"
	"time"

	"botbandit/internal/city"
)

const SaveFile = "data/save.json"

type Inventory struct {
	Clothes  []string `json:"clothes"`
	Cars     []string `json:"cars"`
	Houses   []string `json:"houses"`
	Gadgets  []string `json:"gadgets"`
	Weapons  []string `json:"weapons"`
	Items    []string `json:"items"` // new
}

type Player struct {
	Name         string             `json:"name"`
	Gender       string             `json:"gender"`
	Hair         string             `json:"hair"`
	HairColor    string             `json:"hair_color"`
	Money        int64              `json:"money"`
	Level        int                `json:"level"`
	Exp          int                `json:"exp"`
	Inventory    Inventory          `json:"inventory"`
	Equipped     map[string]string  `json:"equipped"`
	City         string             `json:"city"`
	Energy       int                `json:"energy"`
	Fuel         int                `json:"fuel"`
	MaxFuel      int                `json:"max_fuel"`
	ActiveCar    string             `json:"active_car"`
	Stocks       map[string]int     `json:"stocks"`
	Businesses   map[string]int     `json:"businesses"` // city -> level
	Strength     int                `json:"strength"`
	Agility      int                `json:"agility"`      // new
	Intelligence int                `json:"intelligence"` // new
	Reputation   int                `json:"reputation"`
	Heat         int                `json:"heat"` // police attention 0-100
	WantedLevel  int                `json:"wanted_level"` // 0-5 stars
	HasLicense   bool               `json:"has_license"`
	Pet          string             `json:"pet"`
	Gang         string             `json:"gang"` // new
	GangRank     int                `json:"gang_rank"`
	SkillPoints  int                `json:"skill_points"` // new
	Skills       map[string]int     `json:"skills"`       // new
	Achievements map[string]bool    `json:"achievements"` // new
	Crypto       map[string]float64 `json:"crypto"`       // new
	Materials    map[string]int     `json:"materials"`    // crafting
	QuestProgress map[string]int    `json:"quest_progress"`
	QuestDone    []string           `json:"quest_done"`
	Stats        map[string]int     `json:"stats"`
	LastWork     time.Time          `json:"last_work"`
	LastRob      time.Time          `json:"last_rob"`
	LastDaily    time.Time          `json:"last_daily"`
	LastHeist    time.Time          `json:"last_heist"`
	LastTrain    time.Time          `json:"last_train"`
	CreatedAt    time.Time          `json:"created_at"`
	PlayTime     int64              `json:"play_time"` // seconds
}

func NewPlayer(name string) *Player {
	now := time.Now()
	return &Player{
		Name:         name,
		Gender:       "male",
		Hair:         "short",
		HairColor:    "dark",
		Money:        25000,
		Level:        1,
		Exp:          0,
		Inventory:    Inventory{Clothes: []string{}, Cars: []string{}, Houses: []string{}, Gadgets: []string{}, Weapons: []string{}, Items: []string{}},
		Equipped:     map[string]string{},
		City:         "липецк",
		Energy:       100,
		Fuel:         0,
		MaxFuel:      0,
		ActiveCar:    "",
		Stocks:       make(map[string]int),
		Businesses:   make(map[string]int),
		Strength:     1,
		Agility:      1,
		Intelligence: 1,
		Reputation:   0,
		Heat:         0,
		WantedLevel:  0,
		HasLicense:   false,
		Pet:          "",
		Gang:         "",
		GangRank:     0,
		SkillPoints:  0,
		Skills:       map[string]int{"stealth": 0, "driving": 0, "shooting": 0, "hacking": 0, "negotiation": 0},
		Achievements: make(map[string]bool),
		Crypto:       map[string]float64{"BTC": 0, "ETH": 0, "BANDIT": 0},
		Materials:    map[string]int{"scrap": 0, "electronics": 0, "chemicals": 0, "fabric": 0, "metal": 0},
		QuestProgress: make(map[string]int),
		QuestDone:    []string{},
		Stats:        map[string]int{"work": 0, "rob": 0, "casino": 0, "travel": 0, "heist": 0, "kills": 0},
		LastWork:     time.Time{},
		LastRob:      time.Time{},
		LastDaily:    time.Time{},
		LastHeist:    time.Time{},
		LastTrain:    time.Time{},
		CreatedAt:    now,
		PlayTime:     0,
	}
}

func (p *Player) CityInfo() *city.City {
	c, ok := city.Cities[p.City]
	if !ok {
		return city.Cities["липецк"]
	}
	return c
}

func (p *Player) Price(base int) int {
	return int(float64(base) * p.CityInfo().PriceMult)
}

func (p *Player) AddExp(amount int) []string {
	msgs := []string{}
	p.Exp += amount
	for p.Exp >= p.Level*140 {
		p.Exp -= p.Level * 140
		p.Level++
		p.SkillPoints++
		bonus := int64(12000 + p.Level*800)
		p.Money += bonus
		msgs = append(msgs, fmt.Sprintf("🎉 УРОВЕНЬ %d! +%d₽ и 1 очко навыков", p.Level, bonus))
		if p.Level%5 == 0 {
			p.SkillPoints++
			msgs = append(msgs, "✨ Бонусное очко навыков за каждые 5 уровней!")
		}
	}
	return msgs
}

func (p *Player) CanWork() bool {
	if p.LastWork.IsZero() {
		return true
	}
	return time.Since(p.LastWork) >= 18*time.Second
}

func (p *Player) Work(jobIndex int) (earned int64, msgs []string, jobName string, err string) {
	if p.Energy < 12 {
		return 0, nil, "", "Нет энергии. Отдохни или купи энергетик."
	}
	if !p.CanWork() {
		left := 18 - int(time.Since(p.LastWork).Seconds())
		return 0, nil, "", fmt.Sprintf("Подожди ещё %d сек перед работой", left)
	}

	info := p.CityInfo()
	if jobIndex < 0 || jobIndex >= len(info.Jobs) {
		jobIndex = 0
	}
	job := info.Jobs[jobIndex]

	mult := 1.0 + float64(info.WorkBonus)/200.0
	if info.Special == "factory" {
		mult *= 1.12
	}
	if p.Skills["negotiation"] > 0 {
		mult *= 1.0 + float64(p.Skills["negotiation"])*0.04
	}

	base := float64(job.BasePay + p.Level*220 + p.Strength*160)
	earned = int64(base * mult)
	if p.Businesses[p.City] > 0 {
		earned += int64(p.Businesses[p.City] * 80)
	}

	p.Money += earned
	p.Energy -= 10 + rand.Intn(6)
	if p.Energy < 0 {
		p.Energy = 0
	}
	p.Stats["work"]++
	p.LastWork = time.Now()
	msgs = p.AddExp(28 + job.BasePay/450)
	jobName = job.Name
	return earned, msgs, jobName, ""
}

func (p *Player) CanRob() bool {
	if p.LastRob.IsZero() {
		return true
	}
	return time.Since(p.LastRob) >= 45*time.Second
}

func (p *Player) Rob() (success bool, amount int64, msgs []string, err string) {
	if p.Energy < 20 {
		return false, 0, nil, "Слишком устал для ограбления"
	}
	if !p.CanRob() {
		left := 45 - int(time.Since(p.LastRob).Seconds())
		return false, 0, nil, fmt.Sprintf("Слишком горячо. Подожди %d сек", left)
	}
	if p.WantedLevel >= 4 {
		return false, 0, nil, "Слишком высокий розыск! Спрячься."
	}

	info := p.CityInfo()
	chance := 0.55 + float64(p.Skills["stealth"])*0.06 + float64(p.Agility)*0.02 - float64(info.DangerLevel)*0.04
	if p.Heat > 40 {
		chance -= 0.15
	}
	if chance > 0.92 {
		chance = 0.92
	}
	if chance < 0.18 {
		chance = 0.18
	}

	p.Energy -= 18
	p.LastRob = time.Now()
	p.Stats["rob"]++

	if rand.Float64() < chance {
		base := 1800 + p.Level*350 + info.DangerLevel*900
		amount = int64(float64(base) * (0.7 + rand.Float64()*0.8))
		p.Money += amount
		p.Heat += 8 + info.DangerLevel*3 + rand.Intn(6)
		if p.Heat > 100 {
			p.Heat = 100
		}
		if p.Heat > 60 && rand.Float64() < 0.3 {
			p.WantedLevel++
			if p.WantedLevel > 5 {
				p.WantedLevel = 5
			}
			msgs = append(msgs, fmt.Sprintf("🚨 Розыск повышен до %d★", p.WantedLevel))
		}
		msgs = append(p.AddExp(40+info.DangerLevel*8), msgs...)
		p.CheckAchievement("first_rob")
		return true, amount, msgs, ""
	}

	// fail
	p.Heat += 15 + info.DangerLevel*4
	if p.Heat > 100 {
		p.Heat = 100
	}
	if rand.Float64() < 0.4 {
		p.WantedLevel++
		if p.WantedLevel > 5 {
			p.WantedLevel = 5
		}
		msgs = append(msgs, fmt.Sprintf("🚨 Тебя заметили! Розыск %d★", p.WantedLevel))
	}
	fine := int64(800 + p.Level*200)
	if p.Money >= fine {
		p.Money -= fine
		msgs = append(msgs, fmt.Sprintf("Потерял %d₽ при побеге", fine))
	}
	return false, 0, msgs, "Провал! Пришлось бежать."
}

func (p *Player) Train(stat string) (ok bool, msg string) {
	if p.Energy < 15 {
		return false, "Нет энергии"
	}
	if time.Since(p.LastTrain) < 25*time.Second {
		return false, "Слишком часто. Отдохни."
	}
	info := p.CityInfo()
	if !info.HasGym && stat != "intelligence" {
		return false, "В этом городе нет качалки"
	}

	p.Energy -= 14
	p.LastTrain = time.Now()
	gain := 1
	if rand.Float64() < 0.25 {
		gain = 2
	}

	switch stat {
	case "strength":
		p.Strength += gain
		return true, fmt.Sprintf("💪 Сила +%d (теперь %d)", gain, p.Strength)
	case "agility":
		p.Agility += gain
		return true, fmt.Sprintf("🏃 Ловкость +%d (теперь %d)", gain, p.Agility)
	case "intelligence":
		p.Intelligence += gain
		return true, fmt.Sprintf("🧠 Интеллект +%d (теперь %d)", gain, p.Intelligence)
	default:
		return false, "Неизвестный навык"
	}
}

func (p *Player) Travel(to string, useCar bool) (cost int64, timeSec int, msgs []string, err string) {
	if _, ok := city.Cities[to]; !ok {
		return 0, 0, nil, "Нет такого города"
	}
	if to == p.City {
		return 0, 0, nil, "Ты уже здесь"
	}

	dist := city.GetDistance(p.City, to)
	baseCost := int64(dist * 2.2)
	travelTime := int(dist / 18) // rough minutes -> we use seconds for game

	if useCar {
		if p.ActiveCar == "" {
			return 0, 0, nil, "Нет активной машины"
		}
		if p.Fuel < 8 {
			return 0, 0, nil, "Мало топлива"
		}
		p.Fuel -= 8 + int(dist/80)
		if p.Fuel < 0 {
			p.Fuel = 0
		}
		baseCost = int64(float64(baseCost) * 0.35)
		travelTime = int(float64(travelTime) * 0.55)
		if p.Skills["driving"] > 0 {
			travelTime = int(float64(travelTime) * (1.0 - float64(p.Skills["driving"])*0.04))
		}
	} else {
		// taxi / train
		baseCost = int64(float64(baseCost) * 1.1)
	}

	if p.Money < baseCost {
		return 0, 0, nil, fmt.Sprintf("Нужно %d₽", baseCost)
	}

	p.Money -= baseCost
	p.City = to
	p.Stats["travel"]++
	p.Energy -= 5 + rand.Intn(8)
	if p.Energy < 0 {
		p.Energy = 0
	}

	// heat slowly drops when travelling far
	if dist > 400 {
		p.Heat -= 8
		if p.Heat < 0 {
			p.Heat = 0
		}
	}

	msgs = p.AddExp(15 + int(dist/50))
	return baseCost, travelTime, msgs, ""
}

func (p *Player) ClaimDaily() (reward int64, msgs []string, err string) {
	if !p.LastDaily.IsZero() && time.Since(p.LastDaily) < 20*time.Hour {
		left := 20*time.Hour - time.Since(p.LastDaily)
		return 0, nil, fmt.Sprintf("Ежедневка через %v", left.Round(time.Minute))
	}
	base := int64(3500 + p.Level*400)
	bonus := int64(0)
	if p.Reputation > 20 {
		bonus = int64(p.Reputation * 50)
	}
	reward = base + bonus + int64(rand.Intn(1500))
	p.Money += reward
	p.LastDaily = time.Now()
	p.Energy = 100
	msgs = p.AddExp(50)
	p.CheckAchievement("daily_3")
	return reward, msgs, ""
}

func (p *Player) BuySkill(skill string) (ok bool, msg string) {
	if p.SkillPoints < 1 {
		return false, "Нет очков навыков"
	}
	if _, exists := p.Skills[skill]; !exists {
		return false, "Нет такого навыка. Доступны: stealth, driving, shooting, hacking, negotiation"
	}
	p.SkillPoints--
	p.Skills[skill]++
	return true, fmt.Sprintf("Навык %s повышен до %d", skill, p.Skills[skill])
}

func (p *Player) LowerHeat(amount int) {
	p.Heat -= amount
	if p.Heat < 0 {
		p.Heat = 0
	}
	if p.Heat < 25 && p.WantedLevel > 0 && rand.Float64() < 0.4 {
		p.WantedLevel--
	}
}

func (p *Player) CheckAchievement(id string) {
	if p.Achievements[id] {
		return
	}
	switch id {
	case "first_rob":
		if p.Stats["rob"] >= 1 {
			p.Achievements[id] = true
			p.Money += 5000
			p.SkillPoints++
			fmt.Println("🏆 Достижение: Первое уличное ограбление! +5000₽ +1 навык")
		}
	case "first_heist":
		if p.Stats["heist"] >= 1 {
			p.Achievements[id] = true
			p.Money += 15000
			p.SkillPoints++
			fmt.Println("🏆 Достижение: Первый серьёзный налёт! +15000₽ +1 навык")
		}
	case "big_score":
		p.Achievements[id] = true
		p.Money += 25000
		p.SkillPoints += 2
		fmt.Println("🏆 Достижение: Крупный куш! +25000₽ +2 навыка")
	case "rich":
		if p.Money >= 1_000_000 {
			p.Achievements[id] = true
			p.SkillPoints += 2
			fmt.Println("🏆 Достижение: Миллионер!")
		}
	case "traveler":
		if p.Stats["travel"] >= 10 {
			p.Achievements[id] = true
			fmt.Println("🏆 Достижение: Путешественник")
		}
	case "daily_3":
		p.Achievements[id] = true
	}
}

func (p *Player) StatusLine() string {
	wanted := ""
	if p.WantedLevel > 0 {
		wanted = fmt.Sprintf(" | 🚨 %d★", p.WantedLevel)
	}
	gang := ""
	if p.Gang != "" {
		gang = fmt.Sprintf(" | 🏴 %s", p.Gang)
	}
	return fmt.Sprintf("👤 %s | 💰 %s₽ | ⭐ Ур.%d | ⚡ %d | 🔥 %d%s%s | 📍 %s",
		p.Name, formatMoney(p.Money), p.Level, p.Energy, p.Heat, wanted, gang, p.CityInfo().Name)
}

func formatMoney(m int64) string {
	if m >= 1_000_000_000 {
		return fmt.Sprintf("%.2f млрд", float64(m)/1e9)
	}
	if m >= 1_000_000 {
		return fmt.Sprintf("%.2f млн", float64(m)/1e6)
	}
	if m >= 1000 {
		return fmt.Sprintf("%d тыс", m/1000)
	}
	return fmt.Sprintf("%d", m)
}

func (p *Player) Save() error {
	os.MkdirAll(filepath.Dir(SaveFile), 0755)
	data, err := json.MarshalIndent(p, "", "  ")
	if err != nil {
		return err
	}
	return os.WriteFile(SaveFile, data, 0644)
}

func Load() (*Player, error) {
	data, err := os.ReadFile(SaveFile)
	if err != nil {
		return nil, err
	}
	var p Player
	if err := json.Unmarshal(data, &p); err != nil {
		return nil, err
	}
	if p.Skills == nil {
		p.Skills = map[string]int{"stealth": 0, "driving": 0, "shooting": 0, "hacking": 0, "negotiation": 0}
	}
	if p.Achievements == nil {
		p.Achievements = make(map[string]bool)
	}
	if p.Crypto == nil {
		p.Crypto = map[string]float64{"BTC": 0, "ETH": 0, "BANDIT": 0}
	}
	if p.Materials == nil {
		p.Materials = map[string]int{"scrap": 0, "electronics": 0, "chemicals": 0, "fabric": 0, "metal": 0}
	}
	if p.Stats == nil {
		p.Stats = make(map[string]int)
	}
	if p.Businesses == nil {
		p.Businesses = make(map[string]int)
	}
	if p.Stocks == nil {
		p.Stocks = make(map[string]int)
	}
	return &p, nil
}

func (p *Player) Summary() string {
	s := fmt.Sprintf(`
══════════════════════════════════════
  👤 %s  |  Уровень %d  |  Опыт %d/%d
══════════════════════════════════════
💰 Деньги: %s₽
⚡ Энергия: %d/100
🔥 Тепло: %d  |  Розыск: %d★
💪 Сила %d  |  🏃 Ловкость %d  |  🧠 Интеллект %d
📍 Город: %s
🚗 Машина: %s (топливо %d/%d)
🏴 Банда: %s (ранг %d)
🎯 Очки навыков: %d
`, p.Name, p.Level, p.Exp, p.Level*140,
		formatMoney(p.Money), p.Energy, p.Heat, p.WantedLevel,
		p.Strength, p.Agility, p.Intelligence,
		p.CityInfo().Name, p.ActiveCar, p.Fuel, p.MaxFuel,
		p.Gang, p.GangRank, p.SkillPoints)

	s += "\nНавыки:\n"
	for k, v := range p.Skills {
		s += fmt.Sprintf("  • %s: %d\n", k, v)
	}
	return s
}

// simple helper
func Clamp(v, min, max int) int {
	return int(math.Max(float64(min), math.Min(float64(max), float64(v))))
}