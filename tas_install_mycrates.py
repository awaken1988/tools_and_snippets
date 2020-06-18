from os import system
from os import path
import os

CRATES = (
    "rust_tools/networklist",
    "rust_tools/rust_test_binary"
)

for iCrate in CRATES:
    print("Install Rust Crate {}".format(iCrate))
    
    script_dir = os.path.dirname(os.path.realpath(__file__)) + os.sep
    x = path.join(script_dir, iCrate)
    
    system("cargo install --force --path \"{}/\"".format(x))
  