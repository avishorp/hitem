'use strict'

import { createAction, createReducer } from 'redux-act'

const actions = {
    hit: createAction('hit', (hammerId, hatId) => ({ hammerId: hammerId, hatId: hatId }))
}

export default actions;
