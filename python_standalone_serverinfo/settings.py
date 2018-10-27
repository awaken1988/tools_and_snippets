import mod_listip

MAP_FILES = {"/" : "/index.html"}

FILES = {   "/index.html" : "text/html", 
            "/main.css": "text/css"}

SUBSTITUTE_TEXT = {}
SRVINFO_MODULES = ( mod_listip.ModListip(), mod_listip.ModListip(), mod_listip.ModListip())