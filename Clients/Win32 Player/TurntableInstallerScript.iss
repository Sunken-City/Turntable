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
OutputDir=D:\GitHub\MusicPlayer\Clients\Win32 Player\Installer\

[Files]
Source: "Run_Win32\*"; DestDir: "{app}"; Flags: recursesubdirs
Source: "vc_redist.x86.exe"; DestDir: {tmp}; Flags: deleteafterinstall

[Run]
Filename: {tmp}\vc_redist.x86.exe; Parameters: "/install /passive /norestart"; StatusMsg: Installing VC++ 2015 Redistributables...

[Icons]
Name: "{group}\Turntable"; Filename: "{app}\Turntable.exe"