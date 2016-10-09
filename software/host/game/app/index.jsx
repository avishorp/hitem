'use strict'

import React from 'react';
import ReactDOM from 'react-dom';
import Game from './game'

const KEY_START = 13  // ENTER
const KEY_STOP  = 96  // ~

/*
// Hook to the keyboard events
const body = document.getElementsByTagName('body')[0]
body.addEventListener("keypress", e => {
console.log(e.which)
    if (e.which === KEY_START) 
        store.dispatch(actions.keyStart())
    else if (e.which === KEY_STOP)
        store.dispatch(actions.keyStop())
})
*/

ReactDOM.render(
    <div>
        <Game/>
    </div>
    ,document.getElementById('app'));



