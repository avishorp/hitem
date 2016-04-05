'use strict'

// Endpoint Server

const net = require('net')
const stream = require('stream')
const util = require('util')
const SRBuffer = require('./shiftreg')

const MESSAGE_LENGTH = 7
const MSG_PROLOG = 0x85

function ProtocolParser() {
	stream.Writable.call(this)
	this.msg = new SRBuffer(7)
}

util.inherits(ProtocolParser, stream.Writable)

ProtocolParser.prototype._write = function(chunk, encoding, done) {
	for(let b of chunk) {
		this.msg.lshift(b)
		
		// Check the validity of the message
		if (this.msg.buffer[0]==MSG_PROLOG) {
			const checksum = this._checksum(this.msg.buffer)
			if (this.msg.buffer[MESSAGE_LENGTH-1] != checksum)
				this.emit('checksum_error')
			else 
				this._doMessageParsing(this.msg.buffer)
		}	
	}
}

ProtocolParser.prototype._doMessageParsing = function(message) {
	
}

ProtocolParser.prototype._checksum = function(buf) {
	let i
	let checksum = 0
	for(i=0; i < buf.length-1; i++)
		checksum += buf[i]
		
	return checksum
}

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
		
		parser.on('checksum_error', () => {
			logger.warning("Checksum error from " + socket.remoteAddress)
		})
	})
	
	// Start listening
	srv.listen(options.port)
}



