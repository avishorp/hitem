'use strict'

import React from 'react'
import MainScreen from './main-screen'
import { Store } from './store'
import Emulator from './emulator'

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

        // In case of emulated endpoints, the emulator must be started
        if (emulate)
            setTimeout(() => endpoints.run(), 4000)

    }
    
    render() {
        return <MainScreen store={this.store}/>
    }
}




