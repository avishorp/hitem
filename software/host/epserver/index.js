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
	this.logger = logger
	
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
		parser.on('parse_error', (desc) => {
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

		// Emitted by the parses when a battery level indication is reveived
		parser.on('battery', level => this.handleBattery(level))
				
		socket.on('error', err => this.handleError(socket, err))

		// Pipe all the traffic to the parser
		socket.pipe(parser)				
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


EPManager.prototype.handleError = function(socket, err)
{
	this.logger.info(util.format("Socket error: %s, closing", err.description))
	socket.close()
}

EPManager.prototype.handleBattery = function(level)
{
	this.logger.info(util.format("Battery level is %dmV", level))
}

module.exports = EPManager
