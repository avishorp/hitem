#!/usr/bin/python2

# Config file generator

import ConfigParser
import struct

BOARD_CONF_FORMAT="ii"
WLAN_CONF_FORMAT="30s30si"

# Read the configuration file
config = ConfigParser.ConfigParser()
config.read("gen_config.ini")

# Read the common attributes
ssid=config.get("common", "ssid")
password=config.get("common", "password")
discport=config.getint("common", "discport")

# Create WLAN config file
wlan_file = file("wlan.bin","w")
wlan_file.write(struct.pack(WLAN_CONF_FORMAT, ssid, password, discport))
wlan_file.close()

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
		
	
	data = struct.pack(BOARD_CONF_FORMAT, number, personality_code)
	filename = "config_%d.bin" % number
	f = file(filename, 'w')
	f.write(data)
	f.close()
	

	


