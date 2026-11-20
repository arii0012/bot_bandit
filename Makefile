.PHONY: build run clean test windows linux darwin all

build:
	go build -o bandit .

run:
	go run .

clean:
	rm -f bandit bandit.exe bandit-macos

test:
	go vet ./...
	go build -o /dev/null .

windows:
	GOOS=windows GOARCH=amd64 go build -o bandit.exe .

linux:
	GOOS=linux GOARCH=amd64 go build -o bandit .

darwin:
	GOOS=darwin GOARCH=amd64 go build -o bandit-macos .

all: linux windows darwin
