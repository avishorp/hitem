'use strict'

// Endpoint Server

const net = require('net')
const dgram = require('dgram')
const util = require('util')
const ProtocolParser = require('./parser')


const EPManager = function() {
}

function EPManager(options, logger) {
	logger.info("Starting")

	this.units = {}
	this.lastHitTime = 0
	
	//this.on('welcome', this.handleWelcome)
	//this.on('sync_resp', this.handleSyncResp)
	//this.on('hit')	
	
	// Create a server object
	const srv = net.createServer(function(socket) {
		const EPAddr = socket.remoteAddress
		logger.info("New connection from " + EPAddr)
		
		socket.on('end', () => {
			logger.info("Unit disconnected")
		})
		
		const parser = new ProtocolParser()
		socket.pipe(parser)
		
		// Emitted by the parser when it wants to send a packet to
		// the endpoint
		parser.on('send_command', msg => {
			 socket.write(msg)
			 })
		
		// Emitted by the parser when it encounters an error
		parser.on('error', (desc) => {
			logger.warn(util.format("Parse error on %s: %s", EPAddr, desc))
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
			this.handleSyncResponse()
		})
		
		setTimeout(_ => {parser.setIndication('chirp')}, 3000)
		setTimeout(_ => {parser.syncReq()}, 8000)
		
		//const syncSocket = dgram.createSocket('udp4')
		//syncSocket.setBroadcast(true)
		//setInterval(() => {
		//	// Every second, send a UDP sync request
		//	syncSocket.send("SYNC", 0, 4, options.port, "10.42.0.255")
		//}, 1000)
	})
	
	// Start listening
	srv.listen(options.port)
}



