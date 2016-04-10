'use strict'

import React from 'react'
import { connect } from 'react-redux'
import MainScreen from './main-screen'

export default class Game extends React.Component {
    constructor(props) {
        super(props)
    }
    
    render() {
        return <MainScreen />
    }
}



