'use strict'

// Endpoint Server

const net = require('net')
const stream = require('stream')
const util = require('util')
const SRBuffer = require('./shiftreg')

const MESSAGE_LENGTH = 7
const MSG_PROLOG = 0x85

const MESSAGE_TYPE_WELCOME = 1
const MESSAGE_TYPE_SET_COLOR = 2
const MESSAGE_TYPE_INDICATE = 3
const MESSAGE_TYPE_SYNC_RSP = 4

const MESSAGE_OFFSET_TYPE = 1
const MESSAGE_OFFSET_WELCOME_NUM = 2
const MESSAGE_OFFSET_WELCOME_PERSONALITY = 3

const PERSONALITY_HAT = 1
const PERSONALITY_HAMMER = 2

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
				this.emit('error', 'Incorrect checksum')
			else 
				this._doMessageParsing(this.msg.buffer)
		}	
	}
}

ProtocolParser.prototype._doMessageParsing = function(message) {
	const type = message.readInt8(MESSAGE_OFFSET_TYPE)
	if (type === MESSAGE_TYPE_WELCOME) {
		const body = {
			boardNum: message.readInt8(MESSAGE_OFFSET_WELCOME_NUM),
			personality: message.readInt8(MESSAGE_OFFSET_WELCOME_PERSONALITY)==PERSONALITY_HAMMER?
				'hammer' : 'hat'
		}
		this.emit('welcome', body)
	}
	else if (type === MESSAGE_TYPE_SYNC_RSP) {
		// TODO
	}
	else
		this.emit('error', util.format('Incorrect message type: %d', type))
			
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
		
		parser.on('error', (desc) => {
			logger.warning(util.format("Parse error on %s: %s", socket.remoteAddress, desc))
		})
		
		parser.on('welcome', d => { console.log(d)} )
	})
	
	// Start listening
	srv.listen(options.port)
}



