package city

import "fmt"

type Job struct {
	Name     string
	BasePay  int
	Cooldown int // seconds
}

type City struct {
	ID           string
	Name         string
	Desc         string
	Unique       string
	PriceMult    float64
	WorkBonus    int
	HasCasino    bool
	HasGym       bool
	HasClub      bool
	HasFights    bool
	HasPort      bool
	Special      string
	Jobs         []Job
	BusinessName string
	BusinessCost int
	Venues       []string
	X, Y         float64 // for map
	DangerLevel  int     // 1-5, affects heat & rob success
}

var Cities = map[string]*City{
	"липецк": {
		ID: "липецк", Name: "Липецк", Desc: "Спокойный промышленный город. Всё дешевле.",
		Unique: "Металлургический завод — сверхурочные с высокой оплатой",
		PriceMult: 0.72, WorkBonus: 0, HasCasino: false, HasGym: true, HasClub: false, HasFights: false,
		Special: "factory", DangerLevel: 1,
		Jobs: []Job{{"Завод (смена)", 3800, 22}, {"Грузчик", 2400, 16}, {"Курьер", 1900, 11}},
		BusinessName: "Мороженка", BusinessCost: 8500,
		Venues: []string{"barbershop", "shop", "bar", "factory"},
		X: 0.35, Y: 0.45,
	},
	"воронеж": {
		ID: "воронеж", Name: "Воронеж", Desc: "Студенческий город с казино.",
		Unique: "Студенческий район — лёгкий заработок и дешёвый фастфуд",
		PriceMult: 0.88, WorkBonus: 45, HasCasino: true, HasGym: true, HasClub: false, HasFights: true,
		Special: "park", DangerLevel: 2,
		Jobs: []Job{{"Официант", 2600, 14}, {"Репетитор", 3400, 18}, {"Промоутер", 2100, 11}},
		BusinessName: "Продуктовый", BusinessCost: 12500,
		Venues: []string{"barbershop", "shop", "bar", "casino", "fights"},
		X: 0.32, Y: 0.58,
	},
	"ростов": {
		ID: "ростов", Name: "Ростов-на-Дону", Desc: "Юг, хорошие тачки и «решения вопросов».",
		Unique: "Южный рынок — можно торговать и «решать вопросы»",
		PriceMult: 0.95, WorkBonus: 75, HasCasino: true, HasGym: true, HasClub: true, HasFights: true,
		Special: "south_market", DangerLevel: 3,
		Jobs: []Job{{"Торговец на рынке", 3200, 16}, {"Таксист", 3700, 20}, {"Охрана склада", 3000, 15}},
		BusinessName: "Шаурма-точка", BusinessCost: 11000,
		Venues: []string{"barbershop", "shop", "bar", "casino", "club", "fights"},
		X: 0.25, Y: 0.70,
	},
	"краснодар": {
		ID: "краснодар", Name: "Краснодар", Desc: "Кубань, агро-бизнес и тёплая погода.",
		Unique: "Агробизнес — сезонные подработки на полях",
		PriceMult: 0.80, WorkBonus: 65, HasCasino: false, HasGym: true, HasClub: true, HasFights: false,
		Special: "agro", DangerLevel: 2,
		Jobs: []Job{{"Сбор урожая", 2800, 13}, {"Водитель фуры", 4200, 24}, {"Продавец на ярмарке", 2500, 12}},
		BusinessName: "Фермерский ларёк", BusinessCost: 9500,
		Venues: []string{"barbershop", "shop", "bar", "factory"},
		X: 0.22, Y: 0.82,
	},
	"сочи": {
		ID: "сочи", Name: "Сочи", Desc: "Курорт, тусовки, дорогой отдых.",
		Unique: "Курорт — чаевые от туристов и сезонный буст",
		PriceMult: 1.40, WorkBonus: 130, HasCasino: true, HasGym: true, HasClub: true, HasFights: false,
		Special: "resort", DangerLevel: 2,
		Jobs: []Job{{"Аниматор", 4100, 17}, {"Бармен в отеле", 4500, 19}, {"Гидовство", 5500, 21}},
		BusinessName: "Сувенирный", BusinessCost: 16000,
		Venues: []string{"barbershop", "shop", "bar", "casino", "club"},
		X: 0.18, Y: 0.88,
	},
	"казань": {
		ID: "казань", Name: "Казань", Desc: "Столица Татарстана. Красиво и дорого.",
		Unique: "IT-хаб и сильная диаспора — можно найти «своих»",
		PriceMult: 1.05, WorkBonus: 90, HasCasino: true, HasGym: true, HasClub: true, HasFights: true,
		Special: "it", DangerLevel: 2,
		Jobs: []Job{{"Программист-фриланс", 6000, 25}, {"Менеджер", 3800, 18}, {"Курьер еды", 2800, 12}},
		BusinessName: "Чайхана", BusinessCost: 14000,
		Venues: []string{"barbershop", "shop", "bar", "casino", "club", "fights"},
		X: 0.42, Y: 0.38,
	},
	"нижний": {
		ID: "нижний", Name: "Нижний Новгород", Desc: "Волга, промышленность и история.",
		Unique: "Автозавод и речной порт",
		PriceMult: 0.90, WorkBonus: 55, HasCasino: false, HasGym: true, HasClub: false, HasFights: true,
		Special: "port", DangerLevel: 2,
		Jobs: []Job{{"Сборка на заводе", 3600, 20}, {"Портовый грузчик", 3100, 15}, {"Таксист", 2900, 14}},
		BusinessName: "Автосервис", BusinessCost: 13000,
		Venues: []string{"barbershop", "shop", "bar", "fights"},
		X: 0.48, Y: 0.32,
	},
	"москва": {
		ID: "москва", Name: "Москва", Desc: "Столица. Деньги, власть, опасность.",
		Unique: "Центр всего. Здесь решаются большие дела",
		PriceMult: 1.85, WorkBonus: 200, HasCasino: true, HasGym: true, HasClub: true, HasFights: true,
		Special: "capital", DangerLevel: 5,
		Jobs: []Job{{"Курьер элитных заказов", 7500, 18}, {"Охрана VIP", 9000, 22}, {"Менеджер проектов", 6500, 20}},
		BusinessName: "Кофейня в центре", BusinessCost: 45000,
		Venues: []string{"barbershop", "shop", "bar", "casino", "club", "fights", "darknet"},
		X: 0.55, Y: 0.28,
	},
	"питер": {
		ID: "питер", Name: "Санкт-Петербург", Desc: "Культурная столица. Дорого и красиво.",
		Unique: "Порт, туристы и «серые» схемы",
		PriceMult: 1.55, WorkBonus: 150, HasCasino: true, HasGym: true, HasClub: true, HasFights: true,
		Special: "culture", DangerLevel: 4,
		Jobs: []Job{{"Экскурсовод", 4200, 16}, {"Бармен", 4800, 17}, {"IT-специалист", 7000, 23}},
		BusinessName: "Арт-бар", BusinessCost: 28000,
		Venues: []string{"barbershop", "shop", "bar", "casino", "club", "fights"},
		X: 0.58, Y: 0.18,
	},
	"екатеринбург": {
		ID: "екатеринбург", Name: "Екатеринбург", Desc: "Урал. Промышленность и суровый характер.",
		Unique: "Горнодобыча и сильный криминал",
		PriceMult: 0.98, WorkBonus: 80, HasCasino: true, HasGym: true, HasClub: true, HasFights: true,
		Special: "ural", DangerLevel: 4,
		Jobs: []Job{{"Шахтёр", 4500, 22}, {"Водитель-дальнобойщик", 4800, 20}, {"Охранник", 3200, 14}},
		BusinessName: "Шашлычная", BusinessCost: 12000,
		Venues: []string{"barbershop", "shop", "bar", "casino", "club", "fights"},
		X: 0.62, Y: 0.42,
	},
	"новосибирск": {
		ID: "новосибирск", Name: "Новосибирск", Desc: "Столица Сибири. Наука и холод.",
		Unique: "Академгородок и логистика",
		PriceMult: 0.92, WorkBonus: 70, HasCasino: false, HasGym: true, HasClub: false, HasFights: true,
		Special: "siberia", DangerLevel: 3,
		Jobs: []Job{{"Лаборант", 3800, 18}, {"Логист", 4100, 17}, {"Курьер", 2600, 12}},
		BusinessName: "Пельменная", BusinessCost: 10500,
		Venues: []string{"barbershop", "shop", "bar", "fights"},
		X: 0.72, Y: 0.48,
	},
	"иркутск": {
		ID: "иркутск", Name: "Иркутск", Desc: "Байкал рядом. Спокойно и далеко.",
		Unique: "Туризм на Байкал и лесная промышленность",
		PriceMult: 0.85, WorkBonus: 50, HasCasino: false, HasGym: true, HasClub: false, HasFights: false,
		Special: "baikal", DangerLevel: 2,
		Jobs: []Job{{"Гид по Байкалу", 3900, 16}, {"Лесоруб", 3600, 19}, {"Рыбак", 2800, 14}},
		BusinessName: "Гостиница", BusinessCost: 15000,
		Venues: []string{"barbershop", "shop", "bar"},
		X: 0.82, Y: 0.52,
	},
	"владивосток": {
		ID: "владивосток", Name: "Владивосток", Desc: "Конец России. Море, Азия, возможности.",
		Unique: "Порт, контрабанда и связь с Азией",
		PriceMult: 1.15, WorkBonus: 110, HasCasino: true, HasGym: true, HasClub: true, HasFights: true, HasPort: true,
		Special: "port_asia", DangerLevel: 4,
		Jobs: []Job{{"Портовый рабочий", 4200, 16}, {"Таможенник (серый)", 5500, 20}, {"Рыбак", 3400, 15}},
		BusinessName: "Суши-бар", BusinessCost: 18000,
		Venues: []string{"barbershop", "shop", "bar", "casino", "club", "fights", "port"},
		X: 0.95, Y: 0.55,
	},
}

func GetCityList() []string {
	order := []string{"липецк", "воронеж", "ростов", "краснодар", "сочи", "казань", "нижний", "москва", "питер", "екатеринбург", "новосибирск", "иркутск", "владивосток"}
	return order
}

func GetDistance(from, to string) float64 {
	c1, ok1 := Cities[from]
	c2, ok2 := Cities[to]
	if !ok1 || !ok2 {
		return 1000
	}
	dx := c1.X - c2.X
	dy := c1.Y - c2.Y
	return (dx*dx + dy*dy) * 3500 // approximate km scale
}

func FormatCityInfo(c *City) string {
	return fmt.Sprintf("%s — %s\n%s", c.Name, c.Desc, c.Unique)
}