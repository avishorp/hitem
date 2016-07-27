'use strict'

const dgram = require('dgram')
const util = require('util')
const struct = require('python-struct')
const tftp = require('tftp')
const fileExists = require('file-exists')
const fs = require('fs')
const yamlEval = require('yaml').eval
const md5 = require('md5')
const binary = require('./binary')
const mstream = require('memory-streams')
const path = require('path')


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
const FIRMWARE_DESCRIPTOR_FILENAME = "hitem_fw.yaml"
const OTA_METADATA_BIN = "hitem_ota.bin"

function createDiscoveryResponse(version, firmwareFilename, tftpPort)
{
	let r = new Buffer(42)
	r.fill(0)
	r.write(DISCOVERY_MAGIC)
	r.writeUInt8(version[0], DISCOVERY_MAGIC.length)
	r.writeUInt8(version[1], DISCOVERY_MAGIC.length + 1)
	r.writeUInt8(version[2], DISCOVERY_MAGIC.length + 2)
	r.writeUInt16LE(tftpPort, DISCOVERY_MAGIC.length + 4)
	r.write(firmwareFilename, DISCOVERY_MAGIC.length + 6)
	
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

function createOTAMetadata(descriptorFilename) {
	const directory = path.dirname(descriptorFilename)
	
	// Read and parse the descriptor file
	const descriptor = yamlEval(fs.readFileSync(path.resolve(directory, descriptorFilename)).toString())
	
	// Validate the descriptor
	//  - Must have a "version" field
	if (descriptor.version) {
		const v = descriptor.version
		
		// - The version should be in x.y.z format
		if (!(/[0-9]{1,3}.[0-9]{1,3}.[0-9]{1,3}/.test(v)))
			throw new Error("Invalid version number format")
			
			const versionNumber = versionToNumbers(v) 
			descriptor.version = versionNumber
	} 
	else
		throw new Error("No version field specified")
	
	// - Must have a "files" field
	if (descriptor.files) {
		const files = descriptor.files
		
		// - Iterate through the files, calc their size and md5
		const fileEntries = files.map(filename => {
			let sourceFilename, destFilename
			if (filename.indexOf("=>") == -1) {
				// Source and dest filenames are the same
				sourceFilename = filename
				destFilename = filename
			}
			else {
				// Dest filename is specified
				const sp = filename.split("=>")
				sourceFilename = sp[0].trim()
				destFilename = sp[1].trim()
			}
			// - Check that this is a local file (no path name allowed)
			if (path.dirname(sourceFilename) !== '.')
				throw Error('Filenames in OTA descriptor must be directory-local')
				
			// - Check the the file exists
			const fullSourceFilename = path.resolve(directory, sourceFilename)
			if (!fileExists(fullSourceFilename))
				throw new Error(`File ${sourceFilename} does not exist`)
			
			return {
				sourceFilename: sourceFilename,
				destFilename: destFilename,
				size: fs.statSync(fullSourceFilename).size,
				md5: md5(fs.readFileSync(fullSourceFilename))
			}		
		})
		descriptor.fileEntries = fileEntries
	}
	else
		throw new Error("No files field specified")
		
	// Create the binary representation of the firmware
	// metadata
	const OTAMetadata = new mstream.WritableStream()
	
	// Header
	OTAMetadata.write(DISCOVERY_MAGIC)						// Magic number [4 bytes]
	OTAMetadata.write(binary.UInt8(descriptor.version[0]))  // Version (major) [1 byte]
	OTAMetadata.write(binary.UInt8(descriptor.version[1]))  // Version (minor) [1 byte]
	OTAMetadata.write(binary.UInt8(descriptor.version[2]))  // Version (patch) [1 byte]
	OTAMetadata.write(binary.UInt8(0))                      // Version (reserved) [1 byte]
	OTAMetadata.write(binary.UInt8(descriptor.files.length))// Number of files in the update [1 byte]
	
	// File entries (one for each file)
	descriptor.fileEntries.forEach(fe => {
		OTAMetadata.write(binary.string(fe.sourceFilename, 32)) // Source filename [32 bytes]
		OTAMetadata.write(binary.string(fe.destFilename, 32))   // Destination filename [32 bytes]
		OTAMetadata.write(binary.UInt32LE(fe.size))             // File size [4 bytes]
		OTAMetadata.write(new Buffer(fe.md5, 'hex'))            // MD5 Checksum [16 bytes]
		OTAMetadata.write(binary.UInt16LE(0))					// Reserved [2 bytes ]
	}, this);
	
	return {
		version: descriptor.version,
		binary: OTAMetadata.toBuffer()
	}
}

module.exports = function(options, logger) {

	// Read and parse the firmware metadata
	let OTAMetadata;
	try {
		OTAMetadata = createOTAMetadata(path.resolve(options.firmware.directory, FIRMWARE_DESCRIPTOR_FILENAME))
	}
	catch(e) {
		logger.error(`Failed reading firmware metadata: ${e.message}`)	
	}


	// Create UDP Discovery server
	//////////////////////////////
	// Initiate the UDP discovery socket
	const srv = dgram.createSocket('udp4')

	// Create a server on the socket
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
			
			const r = createDiscoveryResponse(OTAMetadata? OTAMetadata.version : [0, 0, 0], OTA_METADATA_BIN, options.firmware.port)
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

	
	// Create OTA TFTP server
	/////////////////////////
	if (OTAMetadata) {
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
			
			if (req.file === OTA_METADATA_BIN) {
				logger.info(`TFTP: Node ${req.stats.remoteAddress} requested OTA metadata`)

				res.setSize(OTAMetadata.binary.length)
				res.end(OTAMetadata.binary)
				req.close()
			}
			else {
				logger.info(`TFTP: Node ${req.stats.remoteAddress} requested file ${req.file}`)	
			}
		})
	
		tftpSrv.listen()
	}
	else
		logger.warn("No OTA Metadata, TFTP server will not be created")

}



