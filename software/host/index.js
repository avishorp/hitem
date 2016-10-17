'use strict'

const config = require('./config.json')
const discovery = require('./discovery')
const EPServer = require('./epserver').server
const bunyan = require('bunyan')

// Main
//(function() {
	// Create a logger
	const logger = bunyan.createLogger({
		name: "hitem"
		})
	logger.info("Hit'em server starting")
		
	// Discovery service
	discovery(config.discovery, config.endpoint.port, logger.child({'component': 'Discovery'}))
	
	// Endpoint server
	const eps = new EPServer(config.endpoint, logger.child({'component': 'EPServer'}))
//})()
