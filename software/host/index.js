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
	logger.info('Starting')
		
	// Discovery service
	discovery(config.discovery, logger)
	
	// Endpoint server
	const eps = epserver(config.endpoint, logger)
//})()
