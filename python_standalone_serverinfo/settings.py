import mod_listip
import mod_explore_lan
import mod_routing
import mod_disks

MAP_FILES = {"/" : "/index.html"}

FILES = {   "/index.html" : "text/html", 
            "/main.css": "text/css",
            "/external/jquery-3.3.1.js": "text/css"}

SUBSTITUTE_TEXT = {}
SRVINFO_MODULES = ( mod_listip.ModListip(),
                    mod_routing.ModRouting(),
                    mod_disks.ModDisks(),
                    mod_explore_lan.ModExploreLan("10.0.2.0/24")
                    )