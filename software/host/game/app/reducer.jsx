'use strict'

import { fromJS } from 'immutable'
import { combineReducers } from 'redux-immutable'


const initialStateJoin = {
    enoughPlayers: false,
    numPlayers: 0,
    slots: [0, 1, 2, 3, 4, 5, 6, 7].map(n => ({
        color: 'unassigned',
        score: 0,
        hat: null,
        hammer: null
    }))
}

const initialStateCountdown = {
    value: 3
}

const initialStatePlay = {}

const initialStateDeathmatch = {
    player1: null,
    player2: null
}

const initialStateWinner = {}

const initialState = fromJS({
    major: 'join',
    stateJoin: initialStateJoin,
    stateCountdown: initialStateCountdown,
    statePlay: initialStatePlay,
    stateDeathmatch: initialStateDeathmatch,
    stateWinner: initialStateWinner 
})

export default function reducer(state) {
    return initialState;
}

console.log(initialState)