'use strict'

const config = require('./config.json')
const discovery = require('./discovery')
const epserver = require('./epserver')
const bunyan = require('bunyan')

// Main
//(function() {
	// Create a logger
	const logger = bunyan.createLogger({
		name: "hitem"
		})
	logger.info("Hit'em server starting")
		
	// Discovery service
	discovery(config.discovery, logger.child({'component': 'Discovery'}))
	
	// Endpoint server
	const eps = epserver(config.endpoint, logger.child({'component': 'EPServer'}))
//})()
