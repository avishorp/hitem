'use strict'

const { app, BrowserWindow } = require('electron');
const path = require('path')
const util = require('util')
const bunyan = require('bunyan')
const bformat = require('bunyan-format')
//const hitemConfig = require('../config.json')
//const discoveryServer = require('../discovery')
//const EPServer = require('../epserver')

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

/*
// Hit'em Server
////////////////

// Start Discovery Server
discoveryServer(hitemConfig.discovery, logger.child({'component': 'Discovery'}))
	
// Start Endpoint server
const eps = new EPServer(hitemConfig.endpoint, logger.child({'component': 'EPServer'}))
*/

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
    mainWindow.loadURL('file://' + __dirname + '/static/index.html');
    
    // Open the DevTools.
    // mainWindow.webContents.openDevTools();
 
    // Emitted when the window is closed.
    mainWindow.on('closed', function() {
        // Dereference the window object, usually you would store windows
        // in an array if your app supports multi windows, this is the time
        // when you should delete the corresponding element.
        mainWindow = null;
    })
    
    
})




