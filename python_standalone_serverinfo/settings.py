import mod_listip
import mod_explore_lan

MAP_FILES = {"/" : "/index.html"}

FILES = {   "/index.html" : "text/html", 
            "/main.css": "text/css",
            "/external/jquery-3.3.1.js": "text/css"}

SUBSTITUTE_TEXT = {}
SRVINFO_MODULES = ( mod_listip.ModListip(),
                    mod_explore_lan.ModExploreLan("10.0.2.0/24"))