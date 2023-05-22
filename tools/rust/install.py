import os
import sys

build = (
    "binary_generator",
    "network_check",
    #TODO "tftp"
)

for app in build:
    os.system("cargo install --path {}".format(app))

os.chdir(os.path.dirname(sys.argv[0]))