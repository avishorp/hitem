'use strict'

// Endpoint Server

const net = require('net')
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
			logger.warning(util.format("Parse error on %s: %s", socket.remoteAddress, desc))
		})
		
		parser.on('welcome', d => { console.log(d); parser.setColor('white', 50)
		setTimeout(_ => {parser.setIndication('chirp')}, 3000)} )
	})
	
	// Start listening
	srv.listen(options.port)
}



