'use strict'

import util from 'util'
import { fromJS } from 'immutable'
import { combineReducers } from 'redux-immutable'
import { createReducer } from 'redux-act'
import actions from './actions'
import hitemConfig from '../../config.json'
import { knuthShuffle } from 'knuth-shuffle'

const gameConfig = hitemConfig.game

const availableColors = [ 'blue', 'orange', 'purple', 'lgtgreen',
        'turkiz', 'yellow', 'white', 'pink']

const initialState = fromJS({
    major: 'join',
    enoughPlayers: false,
    numPlayers: 0,
    currentSlot: 0,
    slots: [0, 1, 2, 3, 4, 5, 6, 7].map(n => ({
        index: n,
        color: 'unassigned',
        assignedColor: availableColors[n],
        hatId: null,
        hammerId: null,
        score: 0,
        hatColor: null,
        state: 'unassigned',
        hitState: 'none'
    })),
    ready: false,
    countdownVal: null,
    gracePeriod: false,
    cleanList: []
})

const getActiveSlots = state => state.get('slots')
            .filter(v => (v.get('hatId') || (v.get('hatId') === 0)))
            .filter(v => v.get('state') === 'active')
            
const reducer = createReducer({
    // hit
    //////
    [actions.hitJoin]: (state, payload) => {
        const { hatId, hammerId } = payload
        
        // Check if the hat is already assigned
        const assigned = state.get('slots').reduce(
            (prevVal, currentValue) => prevVal | (currentValue.get('hatId') === hatId), false)

        if (assigned) {
            // Hat already assigned, ignore the request
            console.log(util.format("Hat %d already assigned to hammer %d", hatId, hammerId))
            return state
        }
            
        const currentSlot = state.get('currentSlot')
        const color = state.getIn(['slots', currentSlot, 'assignedColor'])
        
        // Check if the game is ready
        const ready = (currentSlot+1) >= gameConfig.minPlayers
        
        // Form the state
        return state
            .update('currentSlot', i => i + 1)
            .setIn(['slots', currentSlot, 'color'], color)
            .setIn(['slots', currentSlot, 'hatId'], hatId)
            .setIn(['slots', currentSlot, 'hammerId'], hammerId)
            .setIn(['slots', currentSlot, 'hatColor'], color)
            .setIn(['slots', currentSlot, 'state'], 'assigned')
            .set('ready', ready)
    },
    
    [actions.setCountdownMode]: (state, payload) => state
        .set('major', 'countdown')
        .set('countdownVal', payload.value),
        
    [actions.setScoreToAll]: (state, payload) => 
        state.update('slots', slots => slots.map(slot => slot.set('score', payload.score))),
        
    [actions.startGame]: (state, payload) =>
        state
            .set('major', 'game')
            .update('slots', slots =>
                slots.map(slot => slot.update('state', currentState => currentState==='unassigned'? 'inactive' : 'active'))),
        
    [actions.colorTransition]: (state, payload) =>
        // Set all active hats to "chirp" signal
        state.update('slots', 
            slots => slots.map(slot => slot.get('state')==='active'? slot.set('hatColor', 'chirp') : slot))
        .set('gracePeriod', true),
        
    [actions.setGameColors]: (state, payload) => {
        // First of all, clean all the previous markings
        let newState = state.update('slots',
            slots => slots.map(
                slot => slot.set('hitState', 'none')//.set('color', state.get('assignedColor'))
            ))
 
        // Calculate all colors that are currently active
 
        const activeSlots = getActiveSlots(state)
        const activeColors = activeSlots
            .map(v => v.get('color'))
            .toJS()
        const activeIndices = activeSlots
            .map(v => v.get('index'))
            .toJS()
            
        const newColors = shuffleColors(activeColors)
        
        let ni = 0
        activeIndices.forEach(i => { newState = newState.setIn(['slots', i, 'hatColor'], newColors[ni]); ni = ni + 1 })
        
        return newState
        // Assign the shuffled colors to the hats
        return state.update('slots',
            slots => activeIndices.reduce(
                (slots, index) => slots.setIn([index, 'hatColor'], newColors.pop()), slots)
            )
    },
        
    [actions.endGracePeriod]: (state, payload) => state.set('gracePeriod', false),
    
    [actions.hitGame]: (state, payload) => {
        const { hammerId, hatId } = payload
        let clean = null
        
        const gameOver = slotState =>
            slotState.set('state', 'gameOver').set('color', 'red').set('hatColor', 'red')

        // Find the color of the hammer
        const hammerSlotIndex = state.get('slots').findIndex(v => v.get('hammerId') === hammerId)
        if (hammerSlotIndex === -1)
            // Hammer not found, ignore
            return state
        let hammerSlot = state.getIn(['slots', hammerSlotIndex])
        if (hammerSlot.get('state') !== 'active')
            // If the slot is not in 'active' state, ignore it
            return state
        const hammerColor = hammerSlot.get('color')
        
        // Find the color of the hat
        const hatSlotIndex = state.get('slots').findIndex(v => v.get('hatId') === hatId)
        if (hatSlotIndex === -1)
            // Hat not found, ignore
            return state
        let hatSlot = state.getIn(['slots', hatSlotIndex])
        if (hatSlot.get('state') !== 'active')
            // If the slot is not in 'active' state, ignore it
            return state
        const hatColor = hatSlot.get('hatColor')
        
        if (hatColor === hammerColor) {
            // Successult hit
            hammerSlot = hammerSlot
                .update('score', val => val + 1)
                .set('hitState', 'green')
            hatSlot = hatSlot
                .update('score', val => val - 1)
                .set('hitState', 'red')
            
            
            if (hatSlot.get('score') <= 0) {
                hatSlot = gameOver(hatSlot) 
                clean = hatSlot.get('index')
            }
        }
        else {
            // Un-successful hit
            // TODO: grace period
            hammerSlot = hammerSlot
                .update('score', val => val - gameConfig.penalty)
                .set('hitState', 'red')
            
            if (hammerSlot.get('score') <= 0) {
                hammerSlot = gameOver(hammerSlot)
                clean = hammerSlot.get('index')
            }
        }
        
        const newState = state
            .setIn(['slots', hammerSlotIndex], hammerSlot)
            .setIn(['slots', hatSlotIndex], hatSlot)
            //.update('cleanList', cleanList => clean!==null? cleanList.append(clean) : cleanList)
            
        // Calculate the number of active players
        const activePlayers = getActiveSlots(newState).size
        //if (activePlayers === 2)                // No deathmatch for mean time - TODO
        //    // Two players left - Deathmatch
        //    return newState.set('major', 'deathmatch')
        if (activePlayers <= 1) {
            // One player left - winner
            return newState
                .set('major', 'gameOver')
                .update(['slots'], slots =>
                    slots.set(slots.findIndex(slot => slog.get('state') === 'active')), 'winner')
        }
        else
            return newState
    },
    
    [actions.reset]: (state, payload) => initialState
    
//    [actions.clean]: (state, payload) => {
//        if (state.get('cleanList').size === 0)
//            // Nothing to clean
//        const i = state.cleanList.
//    }
}, initialState)

const shuffleColors = function(colorList) {
    
    // Return false if the two lists are completely different
    // (i.e. there is no single place that has same color in both)
    const checkColorList = (list1, list2) =>
        list1.map((v,i) => v !== list2[i])
        .reduce((prev, v) => prev && v, true) 

    let newColorList = colorList.map(v => v)
    while(!checkColorList(colorList, newColorList)) {
        newColorList = knuthShuffle(newColorList)
    }   
        
        
    //console.log(colorList)
    //console.log(newColorList)
    return newColorList
}

export default reducer
