# BOT BANDIT — этап 1: аудит структуры

## Результат ревизии

Проверена текущая структура репозитория и список исходников, подключённых в CMakeLists.txt.

На этом этапе файлы не удалялись. Наличие файла вне GAME_SOURCES само по себе не доказывает, что он не используется: заголовки могут подключаться транзитивно или через агрегирующие заголовки. Удаление без проверки ссылок и сборки создало бы риск сломать проект.

## Проверенные узлы

- CMakeLists.txt — targets, compiler flags, platform libraries, runtime resource copying.
- .github/workflows/build.yml — Windows MSVC и Ubuntu Linux CI.
- resources/app.manifest и resources/app.rc — Windows application metadata.
- installer/win_admin.hpp — декларации WinAPI без включения windows.h.
- installer/win_admin.cpp — WinAPI implementation isolated from Raylib UI.
- installer/setup_main.cpp — Raylib installer UI without direct windows.h include.
- tools/prepare_character_assets.py — optional character texture conversion.

## Найденные проблемы до исправлений

1. Оба GitHub Actions job (Windows и Linux) завершались ошибкой на шаге Build.
2. Windows targets создавались без GUI subsystem flag.
3. CMake создавал папку data, но не копировал в неё runtime data.
4. Texture preparation мог создавать пустые PNG placeholders при отсутствии исходных текстур или Pillow.
5. Linux workflow не публиковал собранный бинарник как artifact.

## Ограничения

- В репозитории не обнаружена папка third_party/Square_Character. Конвертация запускается только если исходные материалы добавлены.
- Причины ошибок C++ компиляции нужно извлечь из подробных логов job и исправлять отдельно; успешный CMake configure не означает успешную компиляцию.
