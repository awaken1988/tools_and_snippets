from os import system
from sys import platform
from urllib.request import urlretrieve
import tempfile
import os

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

#-------------------------------
# build rust stuff
#-------------------------------
def cargo_install(aPath):
    cargo_tempdir = tempfile.mkdtemp()
    print("CARGO_TARGET_DIR={}".format(cargo_tempdir))
    os.environ["CARGO_TARGET_DIR"] = cargo_tempdir
    system("cargo install --path {}".format(aPath))
    
rust_tools_base = "../rust_tools/"
for iItem in os.listdir(rust_tools_base):
   fullpath = os.path.join(rust_tools_base, iItem)
   if not os.path.isdir(fullpath):
       continue
   cargo_install(fullpath)