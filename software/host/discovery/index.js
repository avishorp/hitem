'use strict'

const dgram = require('dgram')
const util = require('util')
const struct = require('python-struct')
const tftp = require('tftp')
const fileExists = require('file-exists')
const fs = require('fs')


// Discovery server
//
// This module listens for discovery requests on a specific UDP port
// and responds them
//
// Response format:
// * Magic string ("HTEM")
// * Firmware version number - 4 bytes - major, minor, patch, zero
// * Firmware filename - 32 zero padded string

const DISCOVERY_MAGIC = "HTEM"

function createDiscoveryResponse(versionString, firmwareFilename)
{
	const version = versionToNumbers(versionString)
	
	let r = new Buffer(40)
	r.fill(0)
	r.write(DISCOVERY_MAGIC)
	r.writeUInt8(version[0], DISCOVERY_MAGIC.length)
	r.writeUInt8(version[1], DISCOVERY_MAGIC.length + 1)
	r.writeUInt8(version[2], DISCOVERY_MAGIC.length + 2)
	r.write(firmwareFilename, DISCOVERY_MAGIC.length + 4)
	
	return r
}

function parseDiscoveryRequest(req)
{
	const discoverReqFormat = ">4sbbbxii"
	const discoverReqSize = struct.sizeOf(discoverReqFormat)
	
	if (req.length != discoverReqSize)
		// Incorrect size
		return null
		
	const unpacked = {
		'magic': req.toString('utf-8', 0, 4),
		'fw_version': [req.readUInt8(4), req.readUInt8(5), req.readUInt8(6)],
		'boardId': req.readUInt32LE(8),
		'personality_raw': req.readUInt32LE(12) 
	}	
	
	if (unpacked.magic !== DISCOVERY_MAGIC)
		return null
	else 
		if (unpacked.personality_raw === 1)
			unpacked.personality = 'hat'
		else if (unpacked.personality_raw === 2)
			unpacked.personality = 'hammer'
		else
			// Invalid personality code
			return null
		
	return unpacked
	
}

function versionToNumbers(vstring) {
	const parts = vstring.split('.')
	const errorMessage = "Version number must be in the x.y.z format, each number between 0 and 255"
	
	if (parts.length != 3)
		throw new Error(errorMessage)
		
	return parts.map(ns => {
		const n = parseInt(ns)
		if (isNaN(n) || (n < 0) || (n > 255))
			throw new Error(errorMessage)
		return n
	})
}

function updateTrackingFile(filename, board, personality, version)
{
	let trackingTable = {}
	
	if (fileExists(filename))
		trackingTable = JSON.parse(fs.readFileSync(filename, 'utf8'));
	
	// Update the board entry	
	const entry = {
		'personality': personality,
		'version': version,
		'updated': (new Date()).toJSON(),
		'connected': (new Date()).toJSON()
	}
	trackingTable[board] = entry
	
	// Write the table back
	fs.writeFileSync(filename, JSON.stringify(trackingTable))
	
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
		const req = parseDiscoveryRequest(msg)
		if (req) {
			// Valid message - send a message back
			const address = rinfo.address
			const port = rinfo.port
			
			const r = createDiscoveryResponse(options.firmware.version, options.firmware.filename)
			srv.send(r, 0, r.length, port, address)
			logger.info(`Replied discovery from ${req.personality} ${req.boardId}`)
			
			console.log(options.tracking)
			if (options.tracking) {
				updateTrackingFile(options.tracking, 
					req.boardId, req.personality, req.fw_version)
			}
		}
		else
			logger.warn("Got invalid discovery message from " + rinfo.address)
	})
	
	srv.bind(options.port)

	// Create TFTP server
	const tftpSrv = tftp.createServer({
		host: '0.0.0.0',
		root: options.firmware.directory,
		port: options.firmware.port,
		denyPUT: true
	})
	
	tftpSrv.on('error', err => {
		logger.error("TFTP Server failed initializing")
		console.log(err)
	})
	tftpSrv.on('listening', _ => {
		logger.info(`TFTP Server listening on ${options.firmware.port}`)
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



