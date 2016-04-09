'use strict'

// Endpoint Server

const net = require('net')
const dgram = require('dgram')
const util = require('util')
const ProtocolParser = require('./parser')



function EPManager(options, logger) {
	logger.info("Starting")

	this.units = {}
	this.lastHitTime = 0
	
	// Create a server object
	const srv = net.createServer(socket => {
		const EPAddr = socket.remoteAddress
		logger.info("New connection from " + EPAddr)
		
		socket.on('end', () => {
			logger.info("Unit disconnected")
		})
		
		const parser = new ProtocolParser()

		
		// Emitted by the parser when it wants to send a packet to
		// the endpoint
		parser.on('send_command', msg => {
			 socket.write(msg)
			 })
		
		// Emitted by the parser when it encounters an error
		parser.on('error', (desc) => {
			logger.warn(util.format("Parse error on %s: %s (Message: %s)", 
				EPAddr, desc.description, util.inspect(desc.message)))
		})
		
		// Emitted by the parser when a "WELCOME" message is received from the endpoint
		parser.on('welcome', ep => {
			logger.info(util.format("Unit no. %d of type %s has joined", ep.boardNum,
				ep.personality==='hammer' ? "HAMMER" : "HAT"))
			 parser.setIndication('blimp')
			 this.handleWelcome(EPAddr, parser, ep)
		})
		
		// Emitted by the parser when a sync response message is received
		parser.on('sync_resp', timestamp => {
			this.handleSyncResponse(timestamp)
		})
		
		// Emitted by the parser when the unit is hit
		parser.on('hit', timestamp => {
			this.handleHit(timestamp)
		})

		// Pipe all the traffic to the parser
		socket.pipe(parser)
				
		//setInterval(_ => {parser.syncReq()}, 2000)
		
		//const syncSocket = dgram.createSocket('udp4')
		//syncSocket.setBroadcast(true)
		//setInterval(() => {
		//	// Every second, send a UDP sync request
		//	syncSocket.send("SYNC", 0, 4, options.port, "10.42.0.255")
		//}, 1000)
		//let d = dgram.createSocket('udp4')
		//d.bind(8000, '0.0.0.0')
		
		//d.setBroadcast(true)
		//d.sendto("sync", 0, 4, 24334, '10.42.0.255')
		//d.setBroadcast(true)
	})
	
	// Start listening
	srv.listen(options.port)
}

EPManager.prototype.handleWelcome = function(addr, parser, ep)
{
	
}

EPManager.prototype.handleSyncResponse = function(timestamp)
{
	console.log('Sync response ' + timestamp)	
}

EPManager.prototype.handleHit = function(timestamp)
{
	console.log('hit at ' + timestamp)
}


module.exports = EPManager
