'use strict'

const stream = require('stream')
const util = require('util')
const SRBuffer = require('./shiftreg')

const MESSAGE_LENGTH = 10
const MSG_PROLOG = "HTEM"

const MESSAGE_TYPE_WELCOME = 1
const MESSAGE_TYPE_SET_COLOR = 2
const MESSAGE_TYPE_INDICATE = 3
const MESSAGE_TYPE_SYNC_RSP = 4
const MESSAGE_TYPE_SYNC_REQ = 5
const MESSAGE_TYPE_HIT      = 6
const MESSAGE_TYPE_BATTERY  = 7
const MESSAGE_TYPE_KEEPALIVE = 99

const MESSAGE_OFFSET_TYPE = 4
const MESSAGE_OFFSET_WELCOME_NUM = 5
const MESSAGE_OFFSET_WELCOME_PERSONALITY = 6
const MESSAGE_OFFSET_SYNC_TIMESTAMP = 5
const MESSAGE_OFFSET_HIT_TIMESTAMP = 5
const MESSAGE_OFFSET_BATTERY = 5

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

// Create a list of all available colors and indication
const pureColorList = Object.keys(Colors)
const indicationList = Object.keys(Indications)
const colorList = pureColorList.concat(indicationList)

function ProtocolParser() {
	stream.Writable.call(this)
	this.msg = new SRBuffer(MESSAGE_LENGTH)
}

util.inherits(ProtocolParser, stream.Writable)

ProtocolParser.prototype._write = function(chunk, encoding, done) {
	for(let b of chunk) {
		this.msg.lshift(b)
	
		// Check the validity of the message
		if (this.msg.buffer.slice(0,4).toString()===MSG_PROLOG) {
			const checksum = this._checksum(this.msg.buffer)
			if (this.msg.buffer[MESSAGE_LENGTH-1] != checksum)
				this.emit('parse_error', {
					description: 'Incorrect checksum',
					message: this.msg.buffer 
				})
			else 
				this._doMessageParsing(this.msg.buffer)
		}	
	}
	
	done()
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
		const timestamp = message.readUInt32LE(MESSAGE_OFFSET_SYNC_TIMESTAMP);
		this.emit('sync_resp', timestamp)
	}
	else if (type === MESSAGE_TYPE_HIT) {
		const timestamp = message.readUInt32LE(MESSAGE_OFFSET_SYNC_TIMESTAMP);
		this.emit('hit', timestamp)		
	}
	else if (type === MESSAGE_TYPE_BATTERY) {
		const level = message.readUInt32LE(MESSAGE_OFFSET_BATTERY);
		this.emit('battery', level)				
	}
	else if (type === MESSAGE_TYPE_KEEPALIVE) {
		// Do nothing
		// console.log("Keepalive")
	}
	else
		this.emit('parse_error', {
			description: util.format('Incorrect message type: %d', type),
			message: message
			})
			
}

ProtocolParser.prototype._checksum = function(buf) {
	let i
	let checksum = 0
	for(i=0; i < MESSAGE_LENGTH-1; i++)
		checksum += buf[i]
		
	return (checksum & 0xff)
}

ProtocolParser.prototype.setColor = function(color, intensity) {
	let msg = new Buffer(MESSAGE_LENGTH)
	msg.fill(0)
	msg.write(MSG_PROLOG)
	msg[4] = MESSAGE_TYPE_SET_COLOR
	msg[5] = Colors[color]
	msg[6] = intensity
	msg[MESSAGE_LENGTH-1] = this._checksum(msg)
	
	this.emit('send_command', msg)
}

ProtocolParser.prototype.setIndication = function(indication) {
	let msg = new Buffer(MESSAGE_LENGTH)
	msg.fill(0)
	msg.write(MSG_PROLOG)
	msg[4] = MESSAGE_TYPE_INDICATE
	msg[5] = Indications[indication]
	msg[MESSAGE_LENGTH-1] = this._checksum(msg)
	
	this.emit('send_command', msg)
}

ProtocolParser.prototype.syncReq = function() {
	let msg = new Buffer(MESSAGE_LENGTH)
	msg.fill(0)
	msg.write(MSG_PROLOG)
	msg[4] = MESSAGE_TYPE_SYNC_REQ
	msg[MESSAGE_LENGTH-1] = this._checksum(msg)
	
	this.emit('send_command', msg)
}

module.exports = { 
	parser: ProtocolParser,
	colors: colorList,
	pureColors: pureColorList,
	indications: indicationList
}
