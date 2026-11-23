; BOT BANDIT: UNDERWORLD — Inno Setup script
; Built by GitHub Actions after compiling botbandit.exe

#define MyAppName "BOT BANDIT: UNDERWORLD"
#define MyAppVersion "1.0.0"
#define MyAppPublisher "BotBandit"
#define MyAppExeName "botbandit.exe"

[Setup]
AppId={{8B3C2F1A-9D4E-4A6B-B2C1-7E5F0A9D3C21}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
DefaultDirName={autopf}\BotBanditUnderworld
DefaultGroupName={#MyAppName}
AllowNoIcons=yes
; User can change install folder
DisableDirPage=no
OutputDir=..\dist-installer
OutputBaseFilename=BotBandit-Setup-{#MyAppVersion}
SetupIconFile=
Compression=lzma2
SolidCompression=yes
WizardStyle=modern
PrivilegesRequired=lowest
ArchitecturesInstallIn64BitMode=x64compatible
UninstallDisplayName={#MyAppName}
VersionInfoVersion={#MyAppVersion}

[Languages]
Name: "russian"; MessagesFile: "compiler:Languages\Russian.isl"
Name: "english"; MessagesFile: "compiler:Default.isl"

[Tasks]
Name: "desktopicon"; Description: "Создать ярлык на рабочем столе"; GroupDescription: "Дополнительно:"; Flags: checkedonce
Name: "quicklaunchicon"; Description: "Ярлык в меню Пуск"; GroupDescription: "Дополнительно:"; Flags: checkedonce

[Files]
; Main exe + assets from staged dir (set by CI)
Source: "stage\*"; DestDir: "{app}"; Flags: ignoreversion recursesubdirs createallsubdirs

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"
Name: "{group}\Удалить {#MyAppName}"; Filename: "{uninstallexe}"
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; Tasks: desktopicon

[Run]
Filename: "{app}\{#MyAppExeName}"; Description: "Запустить {#MyAppName}"; Flags: nowait postinstall skipifsilent unchecked

[UninstallDelete]
Type: filesandordirs; Name: "{app}\data"
