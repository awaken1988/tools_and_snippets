from os import system

# *** vscode ***
VSCODE_EXTENSIONS = (
    "ms-vscode.cpptools",
    "ms-python.python",
    "win: ms-vscode.powershell",
    "rust-lang.rust",
    "fabiospampinato.vscode-diff",
    "ryu1kn.partial-diff",
    "slevesque.vscode-hexdump",)

for iVsCodeExt in VSCODE_EXTENSIONS:
    system("code --install-extension " + iVsCodeExt)

# *** git ***
system("git config core.filemode false")
system("git config --global core.autocrlf false")