# Config file generator

import ConfigParser
import struct

FORMAT="ii30s30siiL"

# Read the configuration file
config = ConfigParser.ConfigParser()
config.read("gen_config.ini")

# Read the common attributes
ssid=config.get("common", "ssid")
password=config.get("common", "password")
discport=config.getint("common", "discport")
srvport=config.getint("common", "srvport")
netmask=int(config.get("common","netmask"), 16)

# Iterate through the units, generate config gile per unit
units = config.items("units")
for (snumber, personality) in units:
	number = int(snumber)
	print "Unit number %d is %s" % (number, personality)
	
	if personality == 'hat':
		personality_code = 1
	elif personality == 'hammer':
		personality_code = 2
	else:
		raise ValueError("Personality must be either 'hammer' or 'hat'")
		
	
	data = struct.pack(FORMAT, number, personality_code, ssid, password, discport, srvport, netmask)
	filename = "config_%d.bin" % number
	f = file(filename, 'w')
	f.write(data)
	f.close()
	

	


