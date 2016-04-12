'use strict'

import React from 'react';
import ReactDOM from 'react-dom';
import { createStore } from 'redux'
import { Provider } from 'react-redux'
import immutable from 'immutable'
import reducer from './reducer'
import Game from './game'
import actions from './actions'

const store = createStore(reducer, immutable.Map())

// Connect to EP Server
ipcRenderer.on('ep-event', (event, arg) => {
    store.dispatch(actions.hit(arg.hammerId, arg.hatId))
})
ipcRenderer.send('connect-ep')

ReactDOM.render(
    <Provider store={store}>
        <Game/>
    </Provider>
    ,document.getElementById('app'));

