'use strict'

import { createAction, createReducer } from 'redux-act'
import ReduxThunk from 'redux-thunk'


const hit = createAction('hit', (hammerId, hatId) => ({ hammerId: hammerId, hatId: hatId }))
const setCountdownMode = createAction('setCountdownMode', value => ({ value: value }))

const keyStart = function() {
    return (dispatch, getState) => {
        if ((getState().get('major')==='join') && (getState().get('ready'))) {
            // If wer'e in join state and ready, start countdown
            dispatch(setCountdownMode(3))
            setTimeout(() => dispatch(setCountdownMode(2)), 1000)
            setTimeout(() => dispatch(setCountdownMode(1)), 2000)
        }    
    }
}

const keyStop = createAction('keyStop')


const actions = { hit, setCountdownMode, keyStart, keyStop }

export default actions;

