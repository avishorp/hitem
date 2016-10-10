'use strict'

import React from 'react'
import MainScreen from './main-screen'
import { Store } from './store'
import Emulator from './emulator'

export default class Game extends React.Component {
    constructor(props) {
        super(props)

        this.store = new Store()
        const emulate = true
        if (emulate) {
            this.emulator = new Emulator()

            this.emulator.on('hit', e => {
                console.log(e)
            })

            this.emulator.run()
        }

    }
    
    render() {
        return <MainScreen store={this.store}/>
    }
}




