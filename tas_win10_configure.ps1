$IS_BASE = Read-Host -Prompt "Choco Install BASE Software [y/n]"
$IS_ADVANCED = Read-Host -Prompt "Choco Install ADVANCED Software [y/n]"
$IS_GAMING = Read-Host -Prompt "Choco Install Gaming Software [y/n]"
$IS_DEVELOPER = Read-Host -Prompt "Choco Install Developer Software [y/n]"

[string[]] $PKG = @()

if( "y" -eq $IS_BASE ) {
    $PKG += "chocolateygui"
    $PKG += "notepadplusplus"
    $PKG += "firefox"
    $PKG += "googlechrome"
    $PKG += "7zip"
    $PKG += "vlc"
    $PKG += "python"
    $PKG += "adobereader"
    $PKG += "libreoffice-still"
    $PKG += "gimp"
    $PKG += "inkscape"
    $PKG += "skype"
    $PKG += "teamviewer"
    $PKG += "git"
    $PKG += "putty"
    $PKG += "aimp"
    $PKG += "sysinternals"
    $PKG += "meld"
    $PKG += "tigervnc"
    $PKG += "sandboxie"
}

if( "y" -eq $IS_ADVANCED) {
    $PKG += "nextcloud-client"
    $PKG += "wireshark"
    $PKG += "vscode"
    $PKG += "winmerge"
    $PKG += "hxd"
    $PKG += "dependencywalker"
    $PKG += "nmap"
    $PKG += "curl"
    $PKG += "wget"
    $PKG += "coretemp"
    $PKG += "cpu-z"
    $PKG += "gpu-z"
    $PKG += "filezilla"
    $PKG += "greenshot"
    $PKG += "jdownloader"
    $PKG += "wiztree"
    $PKG += "windirstat"
    $PKG += "openvpn"
    $PKG += "virtualbox"
}

if( "y" -eq $IS_GAMING) {
    $PKG += "dolphin"
    $PKG += "teamspeak"
    $PKG += "discord"
    $PKG += "visualboyadvance"
    $PKG += "snes9x"
    #Steam
    #Origin
    #Battlenet
    #Wise Game Booster
}

if( "y" -eq $IS_DEVELOPER ) {
    $PKG += "tortoisegit"
    $PKG += "teraterm"
}


#Develper
#   Visual Studio

#System encrpytion
#   $PKG += "veracrypt"

#misc
#   Blender
#   Unity
#   Netbalancer	            netbalancer
#   Google Earth	        googleearth	
#   Filezilla | xlight		
#   Treesize Free		
#   Virtual DJ		
#   Windows Terminal		
#   Link Shell Extension		
#   Powershell ISE	                    use vscode instead	
#   mRemoteNG	mremoteng	Base        really slow

#TODO: install chocolatey automatically if it is not avail

Write-Host "Install: " $PKG

foreach( $iPkg in $PKG ) {
    choco install -y $iPkg
}
