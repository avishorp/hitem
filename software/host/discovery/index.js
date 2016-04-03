'use strict'

const dgram = require('dgram')
const util = require('util')

// Discovery server
//
// This module listens for discovery requests on a specific UDP port
// and responds them
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
			srv.send(options.magic, 0, options.magic.length, port, address)
			logger.info("Replied discovery from " + rinfo.address)
		}
		else
			logger.warn("Got invalid discovery message from " + rinfo.address)
	})
	
	srv.bind(options.port)
}



