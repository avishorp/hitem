'use strict'

const dgram = require('dgram')
const util = require('util')
const struct = require('python-struct')
const tftp = require('tftp')


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
	
	
	// Create UDP server
	srv.on('error', err => {
		logger.error(`Socket creation failed: ${err.stack}`)
		srv.close();
	})
	
	
	srv.on('listening', () => {
		logger.info("Discovery Server Listening")
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

	// Create TFTP server
	const tftpSrv = tftp.createServer({
		root: options.firmware.directory,
		port: options.firmware.port,
		denyPUT: true
	})
	
	tftpSrv.on('error', err => {
		logger.error("TFTP Server failed initializing")
		console.log(err)
	})
	tftpSrv.on('listening', _ => {
		logger.info("TFTP Server listening")
	})
	tftpSrv.on('request', (req, res) => {
		req.on('error', err => {
			logger.error("[" + req.stats.remoteAddress + ":" + req.stats.remotePort +
        		"] (" + req.file + ") " + err.message)
		})
		
		logger.info(util.format("TFTP: Node %s requested file %s", req.stats.remoteAddress, req.file))
	})
	
	tftpSrv.listen()

}



