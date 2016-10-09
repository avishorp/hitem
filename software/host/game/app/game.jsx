'use strict'

import React from 'react'
import MainScreen from './main-screen'
import { Store } from './store'

export default class Game extends React.Component {
    constructor(props) {
        super(props)

        this.store = new Store()
    }
    
    render() {
        return <MainScreen store={this.store}/>
    }
}




