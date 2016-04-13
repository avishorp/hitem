'use strict'

import util from 'util'
import { fromJS } from 'immutable'
import { combineReducers } from 'redux-immutable'
import { createReducer } from 'redux-act'
import actions from './actions'
import hitemConfig from '../../config.json'
import { knuthShuffle } from 'knuth-shuffle'

const gameConfig = hitemConfig.game

const initialState = fromJS({
    major: 'join',
    enoughPlayers: false,
    numPlayers: 0,
    colors: [ 'blue', 'orange', 'purple', 'lgtgreen',
        'turkiz', 'yellow', 'white', 'pink'],
    currentSlot: 0,
    slots: [0, 1, 2, 3, 4, 5, 6, 7].map(n => ({
        color: 'unassigned',
        hatId: null,
        hammerId: null,
        score: 0,
        hatColor: null
    })),
    ready: false,
    countdownVal: null,
    gracePeriod: false
})



const reducer = createReducer({
    // hit
    //////
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
        
        // Form the state
        return state
            .update('currentSlot', i => i + 1)
            .update('colors', l => l.pop())
            .setIn(['slots', currentSlot, 'color'], nextColor)
            .setIn(['slots', currentSlot, 'hatId'], hatId)
            .setIn(['slots', currentSlot, 'hammerId'], hammerId)
            .setIn(['slots', currentSlot, 'hatColor'], nextColor)
            .set('ready', ready)
    },
    
    [actions.setCountdownMode]: (state, payload) => state
        .set('major', 'countdown')
        .set('countdownVal', payload.value),
        
    [actions.setScoreToAll]: (state, payload) => 
        state.update('slots', slots => slots.map(slot => slot.set('score', payload.score))),
        
    [actions.startGame]: (state, payload) =>
        state.set('major', 'game'),
        
    [actions.colorTransition]: (state, payload) =>
        state.update('slots', slots => slots.map(slot => slot.set('hatColor', 'chirp')))
        .set('gracePeriod', true),
        
    [actions.setGameColors]: (state, payload) => {
        // Calculate all colors that are currently active
 
        const activeColors = state.get('slots')
            .filter(v => (v.get('hatId') || (v.get('hatID') === 0)))
            .map(v => v.get('color'))
            .toJS()
            
        const newColors = shuffleColors(activeColors)
        
        // Assign the shuffled colors to the hats
        return state.update('slots',
            slots => slots.map(
                (slot, si) => slot.set('hatColor', newColors[si])
            ))
    },
        
    [actions.endGracePeriod]: (state, payload) => state.set('gracePeriod', false)
}, initialState)

const shuffleColors = function(colorList) {
    
    // Return false if the two lists are completely different
    // (i.e. there is no single place that has same color in both)
    const checkColorList = (list1, list2) =>
        list1.map((v,i) => v !== list2[i])
        .reduce((prev, v) => prev && v, true) 
        
    let newColorList = knuthShuffle(colorList.slice(0))
    while(!checkColorList(colorList, newColorList))
        newColorList = knuthShuffle(newColorList)   
        
    return newColorList
}

export default reducer
