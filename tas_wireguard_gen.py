import os
import sys
import subprocess

def strip_mult_spaces(l):
    l=l.replace("\n", "")
    while '  ' in l:
        l=l.replace('  ', ' ')
    return l

def load_file():
    ret = {}
    hosts = []
    f = open(sys.argv[1], "r")
    for i in f.readlines():
        i=strip_mult_spaces(i)
        
        if i.startswith("_ip4"):
            ret["ip4"] = i.split("=")[1]
        elif i.startswith("_ip6"):
            ret["ip6"] = i.split("=")[1]
        elif i.startswith("_server_endpoint"):
            ret["endpoint"] = i.split("=")[1]
        elif i.startswith("_ifname"):
            ret["ifname"] = i.split("=")[1]
        elif i.startswith("#") or i=="":
            continue
        else:        
            i_split = i.split(" ")
            hosts.append({"name": i_split[0], "ip4": i_split[1], "ip6": i_split[2]})
    ret["hosts"] = hosts
    return ret

def generate_load(setting):
    filename_psk="out/psk.key"
    if not os.path.isfile(filename_psk):
        os.system("wg genpsk > {}".format(filename_psk))
    else:
        print("INFO: {} already created".format(filename_psk))
    settings["psk"] = open(filename_psk, "r").readline().replace("\n", "")

    for i, iHost in enumerate(settings["hosts"]):
        filename_priv = "out/{}_{}_priv.key".format(settings["ifname"], iHost["name"])
        filename_pub  = "out/{}_{}_pub.key".format(settings["ifname"], iHost["name"])

        if not os.path.isfile(filename_priv) or not os.path.isfile(filename_priv):
            os.system("wg genkey > {}".format(filename_priv))
            os.system("wg pubkey < {} > {}".format(filename_priv,filename_pub))
        else:
            print("INFO: {} already created".format(filename_priv))
            print("INFO: {} already created".format(filename_pub))

        settings["hosts"][i]["priv"] = open(filename_priv, "r").readline().replace("\n", "")
        settings["hosts"][i]["pub"]  = open(filename_pub, "r").readline().replace("\n", "")

def write_client_conf(settings):
    for i, iHost in enumerate(settings["hosts"]):
        if 0 == i:
            continue

        filename="out/{}_{}.conf".format(settings["ifname"], iHost["name"])
        f=open(filename,"w")

        f.write("[Interface]\n")
        f.write("PrivateKey={}\n".format(iHost["priv"]))

        ip4=settings["ip4"] + iHost["ip4"]
        ip6=settings["ip6"] + iHost["ip6"]

        f.write("Address={}/24,{}/64\n".format(ip4, ip6))

        f.write("\n[Peer]\n")
        f.write("PersistentKeepalive=30\n")
        f.write("PublicKey={}\n".format( settings["hosts"][0]["pub"] ))
        f.write("PresharedKey={}\n".format( settings["psk"]))
        f.write("AllowedIPs={},{}\n".format(
            settings["ip4"]+"0/24",
            settings["ip6"]+"0/64",
         ))
        f.write("EndPoint={}\n".format( settings["endpoint"]))
        f.close()

def write_networkd_conf(settings):
    filename_netdev="out/40_{}.netdev".format(settings["ifname"])
    fliename_network="out/41_{}.network".format(settings["ifname"])
    
    with open(filename_netdev,"w") as f:
        f.write("[NetDev]\n")
        f.write("Name={}\n".format( settings["ifname"] ))
        f.write("Kind=wireguard\n")

        f.write("\n[WireGuard]\n")
        f.write("PrivateKey={}\n".format(  settings["hosts"][0]["priv"] ))
        f.write("ListenPort=51820\n" )
    
        for i, iHost in enumerate(settings["hosts"]):
            if 0 == i:
                continue

            ip4=settings["ip4"] + iHost["ip4"]
            ip6=settings["ip6"] + iHost["ip6"]

            f.write("\n[WireGuardPeer]\n")
            f.write("PublicKey={}\n".format( iHost["pub"] ))
            f.write("PresharedKey={}\n".format( settings["psk"]))
            f.write("AllowedIPs={}\n".format(ip4) )
            f.write("AllowedIPs={}\n".format(ip6) )
           
    with open(fliename_network,"w") as f:
        f.write("[Match]\n")
        f.write("Name={}\n".format( settings["ifname"] ))

        ip4=settings["ip4"] + settings["hosts"][0]["ip4"]
        ip6=settings["ip6"] + settings["hosts"][0]["ip6"]

        f.write("\n[Network]\n")
        f.write("Address={}\n".format( ip4+"/24" ))

        f.write("\n[Route]\n")
        f.write("Gateway={}\n".format( ip4 ))
        f.write("Destination={}\n".format( settings["ip4"]+"0/24" ))

        f.write("\n[Network]\n")
        f.write("Address={}\n".format( ip6+"/64" ))
        
        f.write("\n[Route]\n")
        f.write("Gateway={}\n".format( ip6 ))
        f.write("Destination={}\n".format( settings["ip6"]+"0/64" ))

#TODO: merge this with write_networkd_conf
def write_server_conf(settings):
    filename="out/{}.conf".format(settings["ifname"])
    
    with open(filename,"w") as f:
        ip4=settings["ip4"] + settings["hosts"][0]["ip4"]
        ip6=settings["ip6"] + settings["hosts"][0]["ip6"]

        f.write("[Interface]\n")
        f.write("PrivateKey={}\n".format(  settings["hosts"][0]["priv"] ))
        f.write("ListenPort=51820\n" )
        f.write("Address={},{}\n".format( ip4+"/24", ip6+"64" ))
    
        for i, iHost in enumerate(settings["hosts"]):
            if 0 == i:
                continue

            allow_ip4=settings["ip4"] + iHost["ip4"]
            allow_ip6=settings["ip6"] + iHost["ip6"]

            f.write("\n[Peer]\n")
            f.write("PublicKey={}\n".format( iHost["pub"] ))
            f.write("PresharedKey={}\n".format( settings["psk"]))
            f.write("AllowedIPs={},{}\n".format(allow_ip4, allow_ip6))
           


settings=load_file()
generate_load(settings)
print(settings)

write_client_conf(settings)
write_networkd_conf(settings)
write_server_conf(settings)