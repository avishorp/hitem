'use strict'

import React from 'react';
import ReactDOM from 'react-dom';
import { createStore, applyMiddleware } from 'redux'
import { Provider } from 'react-redux'
import thunk from 'redux-thunk';
import promise from 'redux-promise';
import createLogger from 'redux-logger';
import immutable from 'immutable'
import reducer from './reducer'
import Game from './game'
import EPState from './epstate'
import actions from './actions'

const KEY_START = 13  // ENTER
const KEY_STOP  = 96  // ~

const logger = createLogger({ stateTransformer: s => s.toJS() })
const store = createStore(
  reducer,
  applyMiddleware(thunk, promise, logger),
  
);
//const store = createStore(reducer, immutable.Map())

// Connect to EP Server
ipcRenderer.on('ep-event', (event, arg) => {
    store.dispatch(actions.hit(arg.hammerId, arg.hatId))
})
ipcRenderer.send('connect-ep')

// Hook to the keyboard events
const body = document.getElementsByTagName('body')[0]
body.addEventListener("keypress", e => {
console.log(e.which)
    if (e.which === KEY_START) 
        store.dispatch(actions.keyStart())
    else if (e.which === KEY_STOP)
        store.dispatch(actions.keyStop())
})

ReactDOM.render(
    <Provider store={store}>
    <div>
        <Game/>
        <EPState
            setColorDebug= { (id, color, identity) => {console.log(id);console.log(color);console.log(identity) } }
            setColor = { (id, color, intensity) => { ipcRenderer.send('ep-command', {
                op: 'setColor',
                id: id,
                color: color,
                intensity: intensity
            }) }}
            
            setIndication = { (id, indication) => { ipcRenderer.send('ep-command', {
                id: id,
                op: 'setIndication',
                indication: indication
            }) }}
        />
    </div>
    </Provider>
    ,document.getElementById('app'));



