from os import system
from sys import platform
from urllib.request import urlretrieve
import tempfile

#-------------------------------
# vscode extensions
#-------------------------------
VSCODE_EXTENSIONS = (
    "donjayamanne.githistory",
    "ms-vscode.cpptools",
    "ms-python.python",
    "win: ms-vscode.powershell",
    "rust-lang.rust",
    "fabiospampinato.vscode-diff",
    "jinsihou.diff-tool",
    "ryu1kn.partial-diff",
    "slevesque.vscode-hexdump",
    "ms-vscode-remote.remote-ssh",
    "ms-vscode-remote.remote-ssh-edit")

for iVsCodeExt in VSCODE_EXTENSIONS:
    system("code --install-extension " + iVsCodeExt)
    
#-------------------------------
# git
#-------------------------------
system("git config --global core.filemode false")
system("git config --global core.autocrlf false")
