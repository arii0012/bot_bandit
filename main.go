package main

import (
	"bufio"
	"fmt"
	"os"
	"strings"

	"botbandit/internal/game"
	"botbandit/internal/player"
)

func main() {
	fmt.Print("\033[H\033[2J")

	var p *player.Player
	if loaded, err := player.Load(); err == nil && loaded != nil && loaded.Name != "" {
		p = loaded
		fmt.Printf("✓ Загружено сохранение: %s (ур.%d)\n\n", p.Name, p.Level)
	} else {
		reader := bufio.NewReader(os.Stdin)
		fmt.Print("Как тебя зовут, бандит? > ")
		name, _ := reader.ReadString('\n')
		name = strings.TrimSpace(name)
		if name == "" {
			name = "Бандит"
		}
		p = player.NewPlayer(name)
		fmt.Printf("\nДобро пожаловать, %s!\n", p.Name)
	}

	g := game.New(p)
	g.Run()
}
