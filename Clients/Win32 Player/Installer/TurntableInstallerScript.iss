; Turntable installer

[Setup]
AppName=Turntable
AppVersion=0.1
DefaultDirName={pf64}\Turntable
DefaultGroupName=Turntable
OutputBaseFilename=TurntableInstaller
UninstallDisplayIcon={app}\Turntable.exe
Compression=lzma2
SolidCompression=yes
OutputDir=D:\GitHub\MusicPlayer\Clients\Win32 Player\Installer\

[Files]
Source: "Dawn.exe"; DestDir: "{app}"
Source: "Dawn\*"; DestDir: "{app}\Dawn"; Flags: recursesubdirs
Source: "Engine\*"; DestDir: "{app}\Engine"; Flags: recursesubdirs
Source: "vc_redist.x64.exe"; DestDir: {tmp}; Flags: deleteafterinstall

[Run]
Filename: {tmp}\vc_redist.x64.exe; Parameters: "/install /passive /norestart"; StatusMsg: Installing VC++ 2015 Redistributables...

[Icons]
Name: "{group}\Dawn"; Filename: "{app}\Dawn.exe"