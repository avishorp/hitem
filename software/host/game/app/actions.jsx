'use strict'

import { createAction, createReducer } from 'redux-act'
import ReduxThunk from 'redux-thunk'
import hitemConfig from '../../config.json'

const gameConfig = hitemConfig.game


const hit = createAction('hit', (hammerId, hatId) => ({ hammerId: hammerId, hatId: hatId }))
const setCountdownMode = createAction('setCountdownMode', value => ({ value: value }))

const keyStart = function() {
    return (dispatch, getState) => {
        if ((getState().get('major')==='join') && (getState().get('ready'))) {
            // If wer'e in join state and ready, start countdown
            dispatch(setCountdownMode(3))
            setTimeout(() => dispatch(setCountdownMode(2)), 1000)
            setTimeout(() => dispatch(setCountdownMode(1)), 2000)
            
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


const actions = { hit, setCountdownMode, setScoreToAll, keyStart, keyStop }

export default actions;

