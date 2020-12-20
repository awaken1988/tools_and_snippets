#Requires -RunAsAdministrator

$PKG_BASE=@"
chocolateygui
notepadplusplus
firefox
googlechrome
7zip
vlc
python
adobereader
libreoffice-still
gimp
inkscape
skype
teamviewer
putty
aimp
sysinternals
meld
tightvnc
sandboxie
vscode
syspin
git
totalcommander
avastfreeantivirus
"@

$PKG_TOOLS=@"
nextcloud-client
wireshark
winmerge
hxd
dependencywalker
nmap
curl
wget
coretemp
cpu-z
gpu-z
filezilla
greenshot
jdownloader
wiztree
windirstat
openvpn
virtualbox
"@

$PKG_GAMING=@"
steam
origin
battle.net
dolphin
teamspeak
visualboyadvance
snes9x
"@

$PKG_ALL=@{
    "BASE"=$PKG_BASE;
    "TOOLS"=$PKG_TOOLS;
    "GAMING"=$PKG_GAMING
}

#############################################
# Check if chocolatey ;-) is avail
#############################################
Get-Command -errorAction SilentlyContinue choco 
if( -not $? ) {
    Write-Host "choco not found; Install"
    Invoke-WebRequest https://chocolatey.org/install.ps1 -OutFile c:\chocolatey_install.ps1
    powershell -ExecutionPolicy Bypass -File c:\chocolatey_install.ps1
    throw "reopen powershell"
}

#############################################
# Ask
#############################################
$CHOOSE_PKG=@()

$PKG_ALL.Keys | ForEach-Object {
    $result = Read-host -Prompt "Install Package ${_}? [y/n]"
    if($result -eq "y") {
        $CHOOSE_PKG += $_
    }
}

#############################################
# Install Software
#############################################
foreach($iPkgGroup in $CHOOSE_PKG) {
    foreach( $iPkg in ( $PKG_ALL[$iPkgGroup] -Split [System.Environment]::NewLine) ) {
        Write-Host "Install Package ${iPkg}"
        choco install --yes "${iPkg}"
    }
}

