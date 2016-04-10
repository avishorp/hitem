'use strict'

import React from 'react';
import ReactDOM from 'react-dom';
import { createStore } from 'redux'
import { Provider } from 'react-redux'
import immutable from 'immutable'
import reducer from './reducer'
import Game from './game'

const store = createStore(reducer, immutable.Map())

ReactDOM.render(
    <Provider store={store}>
        <Game/>
    </Provider>
    ,document.getElementById('app'));

