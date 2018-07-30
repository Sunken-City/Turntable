; Turntable installer

[Setup]
AppName=Turntable
AppVersion=0.3
DefaultDirName={pf64}\Turntable
DefaultGroupName=Turntable
OutputBaseFilename=TurntableInstaller
UninstallDisplayIcon={app}\Turntable.exe
Compression=lzma2
SolidCompression=yes
OutputDir=D:\GitHub\Player\Clients\Win32 Player\Installer\
ChangesAssociations=yes

[Files]
Source: "Run_Win32\*"; DestDir: "{app}"; Flags: recursesubdirs
Source: "vc_redist.x86.exe"; DestDir: {tmp}; Flags: deleteafterinstall

[Tasks]
Name: desktopicon; Description: "Create a &desktop icon"; GroupDescription: "Additional icons:"; Flags: checkedonce
Name: associate; Description: "&Associate files"; GroupDescription: "Other tasks:"; Flags: unchecked
Name: associate\mp3; Description: ".mp3"; GroupDescription: "Other tasks"; Flags: unchecked
Name: associate\flac; Description: ".flac"; GroupDescription: "Other tasks"; Flags: unchecked
Name: associate\wav; Description: ".wav"; GroupDescription: "Other tasks"; Flags: unchecked
Name: associate\ogg; Description: ".ogg"; GroupDescription: "Other tasks"; Flags: unchecked

[Run]
Filename: {tmp}\vc_redist.x86.exe; Parameters: "/install /passive /norestart"; StatusMsg: Installing VC++ 2015 Redistributables...

[Icons]
Name: "{group}\Turntable"; Filename: "{app}\Turntable.exe"
Name: "{userdesktop}\Turntable"; Filename: "{app}\Turntable.exe"; Tasks: desktopicon

[Registry]
Root: HKCR; Subkey: "Applications\Turntable.exe"; ValueType: string; ValueName: ""; ValueData: "Program Turntable"; Flags: uninsdeletekey
Root: HKCR; Subkey: "Applications\Turntable.exe"; ValueType: string; ValueName: "FriendlyAppName"; ValueData: "Turntable"
Root: HKCR; Subkey: "Applications\Turntable.exe\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\Turntable.exe,0"
Root: HKCR; Subkey: "Applications\Turntable.exe\shell\Open"; ValueType: string; ValueName: ""; ValueData: "Play with Turntable"
Root: HKCR; Subkey: "Applications\Turntable.exe\shell\Open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\Turntable.exe"" ""%1"""
Root: HKCR; Subkey: "Applications\Turntable.exe\SupportedTypes"; ValueType: string; ValueName: ".mp3"
Root: HKCR; Subkey: "Applications\Turntable.exe\SupportedTypes"; ValueType: string; ValueName: ".flac"
Root: HKCR; Subkey: "Applications\Turntable.exe\SupportedTypes"; ValueType: string; ValueName: ".wav"
Root: HKCR; Subkey: "Applications\Turntable.exe\SupportedTypes"; ValueType: string; ValueName: ".ogg"
Root: HKCR; Subkey: "Applications\Turntable.exe\SupportedTypes"; ValueType: string; ValueName: ".midi"
Root: HKCR; Subkey: "Applications\Turntable.exe\SupportedTypes"; ValueType: string; ValueName: ".mid"

Root: HKCR; Subkey: "Turntable.mp3"; ValueType: string; ValueName: ""; ValueData: "Turntable audio file (.mp3)"; Flags: uninsdeletekey
Root: HKCR; Subkey: "Turntable.mp3\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\Turntable.exe,0"
Root: HKCR; Subkey: "Turntable.mp3\shell"; ValueType: string; ValueName: ""; ValueData: "Open"
Root: HKCR; Subkey: "Turntable.mp3\shell\Open"; ValueType: string; ValueName: ""; ValueData: "Play"
Root: HKCR; Subkey: "Turntable.mp3\shell\Open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\Turntable.exe"" ""%1"""
;Root: HKCU; Subkey: "Software\Microsoft\Windows\CurrentVersion\Explorer\FileExts\.mp3\UserChoice"; ValueType: string; ValueName: "ProgId"; ValueData: "Turntable.mp3"

Root: HKCR; Subkey: "Turntable.flac"; ValueType: string; ValueName: ""; ValueData: "Turntable audio file (.flac)"; Flags: uninsdeletekey
Root: HKCR; Subkey: "Turntable.flac\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\Turntable.exe,0"
Root: HKCR; Subkey: "Turntable.flac\shell"; ValueType: string; ValueName: ""; ValueData: "Open"
Root: HKCR; Subkey: "Turntable.flac\shell\Open"; ValueType: string; ValueName: ""; ValueData: "Play"
Root: HKCR; Subkey: "Turntable.flac\shell\Open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\Turntable.exe"" ""%1"""

Root: HKCR; Subkey: "Turntable.wav"; ValueType: string; ValueName: ""; ValueData: "Turntable audio file (.wav)"; Flags: uninsdeletekey
Root: HKCR; Subkey: "Turntable.wav\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\Turntable.exe,0"
Root: HKCR; Subkey: "Turntable.wav\shell"; ValueType: string; ValueName: ""; ValueData: "Open"
Root: HKCR; Subkey: "Turntable.wav\shell\Open"; ValueType: string; ValueName: ""; ValueData: "Play"
Root: HKCR; Subkey: "Turntable.wav\shell\Open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\Turntable.exe"" ""%1"""

Root: HKCR; Subkey: "Turntable.ogg"; ValueType: string; ValueName: ""; ValueData: "Turntable audio file (.ogg)"; Flags: uninsdeletekey
Root: HKCR; Subkey: "Turntable.ogg\DefaultIcon"; ValueType: string; ValueName: ""; ValueData: "{app}\Turntable.exe,0"
Root: HKCR; Subkey: "Turntable.ogg\shell"; ValueType: string; ValueName: ""; ValueData: "Open"
Root: HKCR; Subkey: "Turntable.ogg\shell\Open"; ValueType: string; ValueName: ""; ValueData: "Play"
Root: HKCR; Subkey: "Turntable.ogg\shell\Open\command"; ValueType: string; ValueName: ""; ValueData: """{app}\Turntable.exe"" ""%1"""

; TODO: Backup old registry keys for file associations and restore them on uninstalling Turntable
Root: HKCR; Subkey: ".mp3"; ValueData: "Turntable.mp3"; ValueType: string;  ValueName: ""; Flags: uninsdeletevalue; Tasks: associate\mp3
Root: HKCR; Subkey: ".flac"; ValueData: "Turntable.flac"; ValueType: string;  ValueName: ""; Flags: uninsdeletevalue; Tasks: associate\flac
Root: HKCR; Subkey: ".wav"; ValueData: "Turntable.wav"; ValueType: string;  ValueName: ""; Flags: uninsdeletevalue; Tasks: associate\wav
Root: HKCR; Subkey: ".ogg"; ValueData: "Turntable.ogg"; ValueType: string;  ValueName: ""; Flags: uninsdeletevalue; Tasks: associate\ogg
