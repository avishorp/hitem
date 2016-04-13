'use strict'

import { createAction, createReducer } from 'redux-act'
import ReduxThunk from 'redux-thunk'
import hitemConfig from '../../config.json'

const gameConfig = hitemConfig.game


const hit = createAction('hit', (hammerId, hatId) => ({ hammerId: hammerId, hatId: hatId }))
const setCountdownMode = createAction('setCountdownMode', value => ({ value: value }))
const startGame = createAction('startGame')

const keyStart = function() {
    return (dispatch, getState) => {
        if ((getState().get('major')==='join') && (getState().get('ready'))) {
            // If wer'e in join state and ready, start countdown
            dispatch(setCountdownMode(3))
            setTimeout(() => dispatch(setCountdownMode(2)), 1000)
            setTimeout(() => dispatch(setCountdownMode(1)), 2000)
            setTimeout(() => { 
                dispatch(startGame())
                dispatch(nextRound())
                }, 3000)
            
            let i;
            for(i=0; i <= gameConfig.startingScore; i++) {
                const score = i
                setTimeout(() => dispatch(setScoreToAll(score)), 100*score)
            }
                
        }    
    }
}

const keyStop = createAction('keyStop')

const setScoreToAll = createAction('setScoreToAll', score => ({ score: score }))

const nextRound = function() {
    return (dispatch, getState) => {
        dispatch(colorTransition())
        setTimeout( () => dispatch(setGameColors()), 1000)
        setTimeout( () => dispatch(endGracePeriod()), gameConfig.gracePeriod)
    }
}

const colorTransition = createAction('colorTransition')

const setGameColors = createAction('setGameColors')

const endGracePeriod = createAction('endGracePeriod')

const actions = { hit, setCountdownMode, setScoreToAll, keyStart, keyStop, startGame,
    colorTransition, setGameColors, endGracePeriod }

export default actions;

