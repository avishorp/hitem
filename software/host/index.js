'use strict'

const config = require('./config.json')
const discovery = require('./discovery')
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
//})()
