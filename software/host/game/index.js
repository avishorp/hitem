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
            
            
            setTimeout(_ => { console.log('sending'); event.sender.send('ep-event', {
                event: 'hit',
                hammerId: 4,
                hatId: 1    
            })}, 30000)
            setTimeout(_ => { console.log('sending'); event.sender.send('ep-event', {
                event: 'hit',
                hammerId: 2,
                hatId: 5    
            })}, 32000)
            setTimeout(_ => { console.log('sending'); event.sender.send('ep-event', {
                event: 'hit',
                hammerId: 0,
                hatId: 3    
            })}, 34000)
            setTimeout(_ => { console.log('sending'); event.sender.send('ep-event', {
                event: 'hit',
                hammerId: 2,
                hatId: 1    
            })}, 36000)
            setTimeout(_ => { console.log('sending'); event.sender.send('ep-event', {
                event: 'hit',
                hammerId: 2,
                hatId: 1    
            })}, 37000)
            setTimeout(_ => { console.log('sending'); event.sender.send('ep-event', {
                event: 'hit',
                hammerId: 4,
                hatId: 7    
            })}, 39000)
            setTimeout(_ => { console.log('sending'); event.sender.send('ep-event', {
                event: 'hit',
                hammerId: 2,
                hatId: 7    
            })}, 40000)
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



