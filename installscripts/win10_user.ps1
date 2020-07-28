#############################################
# Explorer
#############################################
$explorer_key = "HKCU:\Software\Microsoft\Windows\CurrentVersion\Explorer\Advanced"
Set-ItemProperty -Path $explorer_key -Name Hidden       -Value 1
Set-ItemProperty -Path $explorer_key -Name HideFileExt  -Value 0 

#############################################
# Rust
#############################################
$rust_installer = ($env:TEMP+"\rustup-init.exe")
Invoke-WebRequest -Uri https://win.rustup.rs/x86_64 -OutFile $rust_installer
start-process -Wait -NoNewWindow -FilePath $rust_installer -ArgumentList "-y" 
#if we have more rustup commands: RefreshEnv







#############################################
# Taskbar
#############################################
#if( (Get-Command syspin -ErrorAction SilentlyContinue).Count -gt 0 ) {
#    syspin (Get-Command "powershell.exe cd ${USERPROFILE}").Source 5386
#} else {
#    Write-Host "syspin not installed via choco"
#}
