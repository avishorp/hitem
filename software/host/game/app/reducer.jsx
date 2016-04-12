'use strict'

import { fromJS } from 'immutable'
import { combineReducers } from 'redux-immutable'
import { createReducer } from 'redux-act'
import actions from './actions'

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
        score: 0,
        hat: null,
        hammer: null
    }))
})

const initialStateCountdown = fromJS({
    value: 3
})

const initialStatePlay = fromJS({})

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
        const nextColor = state.getIn(['colors', -1])
        const currentSlot = state.get('currentSlot')
        return state
            .update('currentSlot', i => i + 1)
            .update('colors', l => l.pop())
            .setIn(['slots', currentSlot, 'color'], nextColor)
            .setIn(['slots', currentSlot, 'hat'], payload.hatId)
            .setIn(['slots', currentSlot, 'hammer'], payload.hammerId)
            .setIn(['slots', currentSlot, 'score'], 7)
    }
}, initialStateJoin)

const reducer = combineReducers({
    'global': globalReducer,
    'stateJoin': joinReducer
})

export default reducer
