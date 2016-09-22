'use strict'

// Endpoint Server

const net = require('net')
const dgram = require('dgram')
const EventEmitter = require('events').EventEmitter
const util = require('util')
const ProtocolParser = require('./parser')

function EPManager(options, logger) {
	logger.info("Starting")

	this.units = new Map()
	this.lastHit = { time: 0, id: undefined }
	this.logger = logger
    this.hitWindow = options.hitWindow
	
	// Create a server object
	const srv = net.createServer(socket => {
		const EPAddr = socket.remoteAddress
		logger.info("New connection from " + EPAddr)
		
		socket.on('end', () => {
			logger.info("Unit disconnected")
		})
		
		const parser = new ProtocolParser()

		
		// Emitted by the parser when it wants to send a packet to
		// the endpoint
		parser.on('send_command', msg => {
			 socket.write(msg)
			 })
		
		// Emitted by the parser when it encounters an error
		parser.on('parse_error', (desc) => {
			logger.warn(util.format("Parse error on %s: %s (Message: %s)", 
				EPAddr, desc.description, util.inspect(desc.message)))
		})
		
		// Emitted by the parser when a "WELCOME" message is received from the endpoint
		parser.on('welcome', ep => {
			 this.handleWelcome(EPAddr, parser, ep)
		})
		
		
		// Emitted by the parses when a battery level indication is reveived
		parser.on('battery', level => this.handleBattery(level))
				
		socket.on('error', err => this.handleError(socket, err))

		// Pipe all the traffic to the parser
		socket.pipe(parser)				
	})
	
	// Start listening
	srv.listen(options.port)
    
    // Create sync request
    if (options.syncMethod == 'tcp')
        // Set-up cyclic message sending over TCP connection
        setInterval(this._syncAllTcp.bind(this), options.syncInterval)
    else {
        // Create a UDP sync socket
        this.syncSocket = dgram.createSocket('udp4')

        // Bind it (must be done to enable setBroadcast)
        this.syncSocket.bind(options.port)

        // Enable broadcast
        this.syncSocket.setBroadcast(true)

        // Set-up cyclic message sending over UDP
        setInterval(function() { this._syncAllUdp(options.port) }, options.syncInterval)
    }
}

util.inherits(EPManager, EventEmitter)

EPManager.prototype.handleWelcome = function(addr, parser, ep)
{
	const id = ep.boardNum
    const personality = ep.personality

    this.logger.info(util.format("Unit no. %d of type %s has joined", id,
	   personality==='hammer' ? "HAMMER" : "HAT"))
     
    parser.setIndication('blimp')
    
    // Check if the id is already in the unit list
    if (this.units.has(id)) {
        this.logger.warn(util.format("Unit %d re-registered without disconnecting", id))
        this.units.delete(id)
    }
    
    // Search through the list of units to see if there is another unit
    // with the same address
    const k = this._getUnitByAddress(addr)
    if (k) {
        console.warning(util.format("The unit address is already in use, delteing the other (%d)", k))
        this.units.delete(k)
    }
    
    // Add the unit to the list
    let uentry = {
        addr: addr,
        personality: personality,
        offset: 0,
        setColor: (color, intensity) => parser.setColor(color, intensity),
        setIndication: indication => parser.setIndication(indication),
        syncReq: () => parser.syncReq()
    }
    
    // Emitted by the parser when a sync response message is received
	parser.on('sync_resp', timestamp => {
        //console.info(`SYNC %d`, timestamp)
		uentry.offset = timestamp
	})
    
	// Emitted by the parser when the unit is hit
	parser.on('hit', timestamp => {
        const corrected = timestamp - uentry.offset
        
        if ((corrected - this.lastHit.time) <= this.hitWindow)
            this.handleHit(id, this.lastHit.id)
            
        this.lastHit = {
            time: corrected,
            id: id
        }
        console.log(`HIT(%d): %d`, id, corrected)
	})
    
   
    this.units.set(id, uentry)

    // Emit a "join" event
    this.emit('join', {
        id: id,
        personality: personality
    })
}

EPManager.prototype._getUnitByAddress = function(addr)
{
    let r = undefined
    
    this.units.forEach((value, key) => {
        if (value.addr === addr)
            r = key
    })
    
    return r
}

EPManager.prototype._syncAllTcp = function()
{
    this.units.forEach(value => { value.syncReq() })
}

EPManager.prototype._syncAllUdp = function(port)
{
    msg = new Buffer(4);
    msg.fill(0)
    this.syncSocket.send(msg, 0, 4, port, '255.255.255.255')
}


EPManager.prototype.handleHit = function(id1, id2)
{
    const unit1 = this.units.get(id1)
    const unit2 = this.units.get(id2)
    
    // Make sure the IDs are correct
    if (!unit1) {
        this.logger.error(util.format("Hit received by unknow unit ID %d", id1))
        return
    }
    if (!unit2) {
        this.logger.error(util.format("Hit received by unknow unit ID %d", id2))
        return
    }

    // Make sure it's a hat-by-hammer hit
    if (unit1.personality === unit2.personality) {
        this.logger.warn(util.format("Hammer-to-hammer or hat-to-hat hit (%d, %d)", id1, id2))
        return
    }
    
    // Find who is who
    let hammerId, hatId
    if (unit1.personality === 'hammer') {
        hammerId = id1
        hatId = id2
    }
    else {
        hammerId = id2
        hatId = id1
    }
    
    // Finally, generate an event
    this.emit('hit', {
        hammerId: hammerId,
        hatId: hatId
    })
}

EPManager.prototype.setColor = function(id, color, intensity) {
    const unit = this.units.get(id)
    if (!unit) {
        this.logger.warn(util.format("setColor to unknown ID (%d)", id))
    }
    else
        unit.setColor(color, intensity)
}

EPManager.prototype.setIndication = function(id, indication) 
{
    const unit = this.units.get(id)
    if (!unit) {
        this.logger.warn(util.format("setIndication to unknown ID (%d)", id))
    }
    else
        unit.setIndication(indication)
}


EPManager.prototype.handleError = function(socket, err)
{
	this.logger.info(util.format("Socket error: %s, closing", err.description))
	socket.destroy()
}

EPManager.prototype.handleBattery = function(level)
{
	this.logger.info(util.format("Battery level is %dmV", level))
}

module.exports = EPManager
