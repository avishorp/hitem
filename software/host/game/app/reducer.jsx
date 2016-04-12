'use strict'

import util from 'util'
import { fromJS } from 'immutable'
import { combineReducers } from 'redux-immutable'
import { createReducer } from 'redux-act'
import actions from './actions'
import hitemConfig from '../../config.json'

const gameConfig = hitemConfig.game

const initialStateGlobal = fromJS({
    major: 'join'
})

const initialStateJoin = fromJS({
    enoughPlayers: false,
    numPlayers: 0,
    colors: [ 'blue', 'orange', 'purple', 'lgtgreen',
        'turkiz', 'yellow', 'white', 'pink'],
    currentSlot: 0,
    slots: [0, 1, 2, 3, 4, 5, 6, 7].map(n => ({
        color: 'unassigned',
        hatId: null,
        hammerId: null
    })),
    ready: false
})

const initialStateCountdown = fromJS({
    value: 3
})

const initialStatePlay = fromJS({
    slots: [0, 1, 2, 3, 4, 5, 6, 7].map(n => ({
        state: 'playing',
        score: 5
    }))
})

const initialStateDeathmatch = fromJS({
    player1: null,
    player2: null
})

const initialStateWinner = fromJS({})

const initialState = fromJS({
    major: 'join',
    stateJoin: initialStateJoin,
    stateCountdown: initialStateCountdown,
    statePlay: initialStatePlay,
    stateDeathmatch: initialStateDeathmatch,
    stateWinner: initialStateWinner 
})


const globalReducer = createReducer({
    
}, initialStateGlobal)

const joinReducer = createReducer({
    [actions.hit]: (state, payload) => {
        const { hatId, hammerId } = payload
        
        // Check if the hat is already assigned
        const assigned = state.get('slots').reduce(
            (prevVal, currentValue) => prevVal | (currentValue.get('hatId') === hatId), false)

        if (assigned) {
            // Hat already assigned, ignore the request
            console.log(util.format("Hat %d already assigned to hammer %d", hatId, hammerId))
            return state
        }
            
        const nextColor = state.getIn(['colors', -1])
        const currentSlot = state.get('currentSlot')
        
        // Check if the game is ready
        const ready = (currentSlot+1) >= gameConfig.minPlayers
        console.log(currentSlot)
        console.log( gameConfig.minPlayers)
        console.log(ready)
        
        // Form the state
        return state
            .update('currentSlot', i => i + 1)
            .update('colors', l => l.pop())
            .setIn(['slots', currentSlot, 'color'], nextColor)
            .setIn(['slots', currentSlot, 'hatId'], hatId)
            .setIn(['slots', currentSlot, 'hammerId'], hammerId)
            .set('ready', ready)
    }
}, initialStateJoin)

const reducer = combineReducers({
    'global': globalReducer,
    'stateJoin': joinReducer
})

export default reducer
