'use strict'

// Endpoint Server

const net = require('net')
const dgram = require('dgram')
const util = require('util')
const ProtocolParser = require('./parser')


module.exports = function(options, logger) {
	logger.info("Starting")
	
	// Create a server object
	const srv = net.createServer(function(socket) {
		logger.info("New connection from " + socket.remoteAddress)
		
		socket.on('end', () => {
			logger.info("Unit disconnected")
		})
		
		const parser = new ProtocolParser()
		socket.pipe(parser)
		
		parser.on('send_command', msg => {
			 socket.write(msg)
			 })
		
		parser.on('error', (desc) => {
			logger.warn(util.format("Parse error on %s: %s", socket.remoteAddress, desc))
		})
		
		parser.on('welcome', ep => {
			logger.info(util.format("Unit no. %d of type %s has joined", ep.boardNum,
				ep.personality==='hammer' ? "HAMMER" : "HAT"))
			 parser.setColor('white', 50)
		})
		
		parser.on('sync_resp', d => { console.log(d) })
		
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


