'use strict'

const dgram = require('dgram')
const util = require('util')
const struct = require('python-struct')

// Discovery server
//
// This module listens for discovery requests on a specific UDP port
// and responds them
//
// Response format:
// * Magic string ("HTEM")
// * Firmware version number - 4 bytes - major, minor, patch, zero
// * Firmware filename - 32 zero padded string

function createDiscoveryResponse(magic, versionString, firmwareFilename)
{
	const version = versionToNumbers(versionString)
	return struct.pack("4sbbbx32s", [
		magic, version[0], version[1], version[2], firmwareFilename
	])
}

function versionToNumbers(vstring) {
	const parts = vstring.split('.')
	const errorMessage = "Version number must be in the x.y.z format, each number between 0 and 255"
	
	if (parts.length != 3)
		throw new Error(errorMessage)
		
	return version = map(ns => {
		n = parseInt(ns)
		if (isNaN(n) || (n < 0) || (n > 255))
			throw new Error(errorMessage)
	}, parts)
}

module.exports = function(options, logger) {
	// Initiate the UDP discovery socket
	const srv = dgram.createSocket('udp4')
	
	srv.on('error', err => {
		logger.error(`Socket creation failed: ${err.stack}`)
		srv.close();
	})
	
	srv.on('listening', () => {
		logger.info("Listening")
	})
	
	srv.on('message', (msg, rinfo) => {
		if (msg.toString().startsWith(options.magic)) {
			// Valid message - send a message back
			const address = rinfo.address
			const port = rinfo.port
			
			const r = createDiscoveryResponse(options.magic, options.firmware.version, options.firmware.filename)
			srv.send(r, 0, r.length, port, address)
			logger.info("Replied discovery from " + rinfo.address)
		}
		else
			logger.warn("Got invalid discovery message from " + rinfo.address)
	})
	
	srv.bind(options.port)
}



