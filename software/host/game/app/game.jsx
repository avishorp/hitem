'use strict'

import React from 'react'
import MainScreen from './main-screen'
import CountdownScreen from './countdown-screen'
import { Store, GAME_STATE } from './store'
import Emulator from './emulator'
import { observer } from "mobx-react";

const KEY_START = 13  // ENTER
const KEY_STOP  = 96  // ~

@observer
export default class Game extends React.Component {
    constructor(props) {
        super(props)

        // Instantiate a store
        this.store = new Store()

        // Instantiate an EPServer/emulator
        const emulate = true

        let endpoints
        if (emulate)
            endpoints = new Emulator()
        else {
            // TODO: Instantiate an EPServer
        }

        // Tie the EPServer events to store actions
        endpoints.on('hit', e => this.store.actionHit(e))
        endpoints.on('leave', e => {})

        // TODO: I wonder whether this is the correct place ...
        // Hook to the keyboard events
        const body = document.getElementsByTagName('body')[0]
        body.addEventListener("keypress", e => {
            console.log(e.which)
            if (e.which === KEY_START) 
                this.store.actionStartButton()
            else if (e.which === KEY_STOP)
                this.store.actionStopButton()
})

        // In case of emulated endpoints, the emulator must be started
        if (emulate)
            setTimeout(() => endpoints.run(), 4000)

    }
    
    render() {
        const store = this.store
        let screen;

        switch(store.gameState) {
            case GAME_STATE.JOIN:
                screen = <MainScreen mode='join' slots={store.slots}/>
                break;

            case GAME_STATE.COUNTDOWN:
                screen = <CountdownScreen value={store.countdownValue}/>
                break;

            case GAME_STATE.GAME:
                screen = <MainScreen mode='game' slots={store.slots}/>
                break;

            default:
                console.error("Unknow game state")
        }

        return screen
    }
}




