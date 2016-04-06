'use strict'

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

const Colors = {
	'none': 0,
	'red': 1,
	'green': 2,
	'blue': 3,
	'orange': 4,
	'purple': 5,
	'lgtgreen': 6,
	'turkiz': 7,
	'yellow': 8,
	'white': 9,
	'pink': 10
}

const Indications = {
	'red_pulse': 2,
	'green_pulse': 3,
	'blimp': 4,
	'chirp': 5
}

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

ProtocolParser.prototype.setColor = function(color, intensity) {
	let msg = new Buffer(MESSAGE_LENGTH)
	msg[0] = MSG_PROLOG
	msg[1] = MESSAGE_TYPE_SET_COLOR
	msg[2] = Colors[color]
	msg[3] = intensity
	msg[MESSAGE_LENGTH-1] = this._checksum(msg)
	
	this.emit('send_command', msg)
}

ProtocolParser.prototype.setIndication = function(indication) {
	let msg = new Buffer(MESSAGE_LENGTH)
	msg[0] = MSG_PROLOG
	msg[1] = MESSAGE_TYPE_INDICATE
	msg[2] = Indications[indication]
	msg[MESSAGE_LENGTH-1] = this._checksum(msg)
	
	this.emit('send_command', msg)
}

module.exports = ProtocolParser
