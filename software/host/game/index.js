'use strict'

const electron = require('electron');
const webpack = require('webpack')
const path = require('path')
const bunyan = require('bunyan')

const app = electron.app;  // Module to control application life.
const BrowserWindow = electron.BrowserWindow;  // Module to create native browser window.

// Keep a global reference of the window object, if you don't, the window will
// be closed automatically when the JavaScript object is garbage collected.
var mainWindow = null;

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
electron.ipcMain.on('connect-ep', event => {
    console.log('connected')
    setTimeout(_ => { console.log('1'); event.sender.send('ep-event', 1)}, 120000)
//    setTimeout(_ => { console.log('2'); event.sender.send('ep-event', {hammer: 1, hat: 2})}, 5000)
//    setTimeout(_ => { console.log('3'); event.sender.send('ep-event')}, 10000)
})    
    // Open the DevTools.
    mainWindow.webContents.openDevTools();

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
            mainWindow.reload()
        };
    });
    
})




