'use strict'

const electron = require('electron');
const webpack = require('webpack')
const path = require('path')
const util = require('util')
const bunyan = require('bunyan')
const bformat = require('bunyan-format')
const hitemConfig = require('../config.json')
const discoveryServer = require('../discovery')
const EPServer = require('../epserver')
const pickRandom = require('pick-random')

const app = electron.app;  // Module to control application life.
const BrowserWindow = electron.BrowserWindow;  // Module to create native browser window.

// Keep a global reference of the window object, if you don't, the window will
// be closed automatically when the JavaScript object is garbage collected.
var mainWindow = null;

// Create a logger
//////////////////
const logFormat = bformat({
    outputMode: 'short',
    levelInString: true
})
let logger = bunyan.createLogger({
    name: 'hitem', 
    stream: logFormat,
    level: 'debug' 
    });


// Hit'em Server
////////////////

// Start Discovery Server
discoveryServer(hitemConfig.discovery, logger.child({'component': 'Discovery'}))
	
// Start Endpoint server
const eps = new EPServer(hitemConfig.endpoint, logger.child({'component': 'EPServer'}))

// Quit when all windows are closed.
app.on('window-all-closed', function() {
    app.quit();
});

app.on('ready', function() {
    // Create the browser window.
    mainWindow = new BrowserWindow({
        titleBarStyle: "hidden", 
        width: 1200, 
        height: 900
        });
    mainWindow.setMenuBarVisibility(false)

    // and load the index.html of the app.
    //mainWindow.loadURL('file://' + __dirname + '/static/index.html');
    
    // Create an IPC event handler to communicate with EP Server
    electron.ipcMain.on('connect-ep', event => {
        logger.info('Connected to EP server')
        
        eps.on('hit', e => {
            console.log('hit')
            event.sender.send('ep-event', {
                event: 'hit',
                hammerId: e.hammerId,
                hatId: e.hatId
            })
        })
        
        const emulation = true
        if (emulation) {
            
            const hammers = [0, 2, 4, 6]
            const hats = [1, 3, 5, 7]    
            setTimeout(_ => { console.log('sending'); event.sender.send('ep-event', {
                event: 'hit',
                hammerId: 0,
                hatId: 1    
            })}, 10000)
            setTimeout(_ => { console.log('sending'); event.sender.send('ep-event', {
                event: 'hit',
                hammerId: 2,
                hatId: 3    
            })}, 15000)
            setTimeout(_ => { console.log('sending'); event.sender.send('ep-event', {
                event: 'hit',
                hammerId: 4,
                hatId: 5    
            })}, 20000)
            setTimeout(_ => { console.log('sending'); event.sender.send('ep-event', {
                event: 'hit',
                hammerId: 4,
                hatId: 3    
            })}, 22000)
            setTimeout(_ => { console.log('sending'); event.sender.send('ep-event', {
                event: 'hit',
                hammerId: 6,
                hatId: 7    
            })}, 25000)
            
            setTimeout(() => {
                let events = 50
                let generator = setInterval(() => {
                    const hammer = pickRandom(hammers)[0]
                    const hat = pickRandom(hats)[0]
                    console.log(util.format("Emulating hit hat=%d hammer=%d", hat, hammer))
                    event.sender.send('ep-event', {
                        event: 'hit',
                        hammerId: hammer,
                        hatId: hat
                    })
                    events = events - 1
                    if (events === 0)
                        clearInterval(generator)    
                }, 1500)
                
            }, 30000)
            
        }

    })
    
    electron.ipcMain.on('ep-command', (event, args) => {
        if (args.op === 'setColor') {
            console.log(util.format("Setting color of %d to %s", args.id, args.color))
            eps.setColor(args.id, args.color, args.intensity)
        }
        else if (args.op === 'setIndication') {
            console.log(util.format("Setting indication of %d to %s", args.id, args.indication))
            eps.setIndication(args.id, args.indication)
        }    
    })
        
    // Open the DevTools.
   // mainWindow.webContents.openDevTools();

    // Emitted when the window is closed.
    mainWindow.on('closed', function() {
        // Dereference the window object, usually you would store windows
        // in an array if your app supports multi windows, this is the time
        // when you should delete the corresponding element.
        mainWindow = null;
    })


    // Set-up webpack
    const webpackConfig = require(path.resolve(__dirname, 'webpack.config.js'))
    const webpackConfigCompiled = webpack(webpackConfig)
    webpackConfigCompiled.watch({}, function(err, stats) {
        if (err)
            console.log(err)
        else {
            console.log(stats.toString({colors: true}))
            mainWindow.loadURL('file://' + __dirname + '/static/index.html');
            //mainWindow.reload()
        };
    });
    
})




