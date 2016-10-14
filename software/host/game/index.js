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
const config = require('../config.js')

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
discoveryServer(hitemConfig.discovery, config.endpoint.port, logger.child({'component': 'Discovery'}))
	
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




