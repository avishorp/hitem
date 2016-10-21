'use strict'

// Endpoint Server

const net = require('net')
const dgram = require('dgram')
const EventEmitter = require('events').EventEmitter
const util = require('util')
const ProtocolParser = require('./parser')

const defaultCalibration = {
    hitThreshold: 2400
}

function EPManager(options, calibration, logger) {
	logger.info("Starting")

	this.units = new Map()
	this.lastHit = { time: 0, id: undefined }
	this.logger = logger
    this.hitWindow = options.hitWindow
    this.calibration = calibration
	
	// Create a server object
	const srv = net.createServer(socket => {
		const EPAddr = socket.remoteAddress
		logger.info("New connection from " + EPAddr)
		
        // Enable keepalive to detect endpoind disconnection
        socket.setTimeout(1000);
            
		const parser = new ProtocolParser.parser()
		
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

        socket.on('timeout', _ => { this.removeUnit(this._getUnitByAddress(EPAddr)) })

		socket.on('end', () => {
			logger.log("Unit disconnected")
            this.removeUnit(this._getUnitByAddress(EPAddr))
		})
		
		socket.on('close', () => {
			logger.log("socket closed")
            this.removeUnit(this._getUnitByAddress(EPAddr))
		})
		

		// Pipe all the traffic to the parser
		socket.pipe(parser)				
	})
	
	// Start listening
	srv.listen(options.port)
    
    // Create sync request
    if (options.syncMethod === 'tcp')
        // Set-up cyclic message sending over TCP connection
        setInterval(this._syncAllTcp.bind(this), options.syncInterval)
    else if (options.syncMethod === 'udp') {
        // Create a UDP sync socket
        this.syncSocket = dgram.createSocket('udp4')

        // Bind it (must be done to enable setBroadcast), and enable broadcast when
        // binding has completed
        this.syncSocket.bind(options.port, () => { this.syncSocket.setBroadcast(true) })

        // Set-up cyclic message sending over UDP
        const syncAllUdpBinded = this._syncAllUdp.bind(this)
        setInterval(function() { syncAllUdpBinded(options.port) }, options.syncInterval)
    }
}

util.inherits(EPManager, EventEmitter)

EPManager.prototype.removeUnit = function(uid) {
    console.log(`RemoveUnit %d`, uid)
    // Make sure the unit exists in the list
    const unit = this.units.get(uid)
    if (!unit) {
        this.logger.warn(util.format("Trying to delete non-existing unit (%d)", uid))
        return
    }

    // Delete it
    this.units.delete(uid)

    // Notify
    this.emit("leave", uid)
}

EPManager.prototype.handleWelcome = function(addr, parser, ep)
{
	const id = ep.boardNum
    const personality = ep.personality

    this.logger.info(util.format("Unit no. %d of type %s has joined", id,
	   personality==='hammer' ? "HAMMER" : "HAT"))
     
    parser.setIndication('blimp')

    // Apply unit level calibration
    let threshold
    if ((this.calibration[id]) && (this.calibration[id].hitThreshold))
        threshold = this.calibration[id].hitThreshold
    else
        threshold = defaultCalibration.hitThreshold
    parser.setThreshold(threshold)
    
    // Check if the id is already in the unit list
    if (this.units.has(id)) {
        this.logger.warn(util.format("Unit %d re-registered without disconnecting", id))
        this.removeUnit(id)
    }
    
    // Search through the list of units to see if there is another unit
    // with the same address
    const k = this._getUnitByAddress(addr)
    if (k) {
        console.warning(util.format("The unit address is already in use, delteing the other (%d)", k))
        this.remove(k)
    }
    
    // Add the unit to the list
    let uentry = {
        id: id,
        addr: addr,
        personality: personality,
        offset: 0,
        setColor: (color, intensity) => parser.setColor(color, intensity),
        setIndication: indication => parser.setIndication(indication),
        setThreshold: (threshold, debounce) => parser.setThreshold(threshold, debounce),
        syncReq: () => parser.syncReq()
    }
    
    // Emitted by the parser when a sync response message is received
	parser.on('sync_resp', timestamp => {
        //console.info(`ID %d SYNC %d delta %d`, id, timestamp, timestamp-uentry.offset)
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
    const msg = new Buffer(4);
    msg.fill(0)
    // TODO: Detect broadcast address somehow
    this.syncSocket.send(msg, 0, 4, port, '10.42.0.255')// '255.255.255.255')
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
  
    // Find who is who
    let hammerId, hatId
    if (unit1.personality === 'hammer' && unit2.personality === 'hat') {
        hammerId = id1
        hatId = id2
    }
    else if (unit1.personality === 'hat' && unit2.personality === 'hammer'){
        hammerId = id2
        hatId = id1
    }
    else
        // Hat-by-Hat or Hammer-by-Hammer, ignore
        return
    
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
    else {
        if (ProtocolParser.pureColors.indexOf(color) >= 0)
            unit.setColor(color, intensity)
        else
            unit.setIndication(color)
    }   
}

EPManager.prototype.setThreshold = function(id, threshold) {
    const unit = this.units.get(id)
    if (!unit) {
        this.logger.warn(util.format("setThreshold to unknown ID (%d)", id))
    }

    unit.setThreshold(threshold)   
}


EPManager.prototype.handleError = function(socket, err)
{
	this.logger.info(util.format("Socket error: %s, closing", err.description))
	socket.destroy()
}

EPManager.prototype.handleBattery = function(data)
{
	this.logger.info(util.format("Battery level is %dmV (raw=%d)", data.level, data.raw))
}

module.exports = { 
    server: EPManager,
    colors: ProtocolParser.colors
}
