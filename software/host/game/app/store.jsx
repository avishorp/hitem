import { observer, computed } from "mobx-react";
import { observable } from "mobx"
import autobind from 'autobind'

// Slot states
export const SLOT_STATE = {
    UNASSIGNED: 'unassigned',
    ASSIGNED: 'assigned',
    ACTIVE: 'active',
    INACTIVE: 'inactive',
    GAME_OVER: 'gameOver'
}

// Game states
export const GAME_STATE = {
    JOIN: 'join',
    COUNTDOWN: 'countdown',
    GAME: 'game',
    DEATHMATCH: 'deathmatch',
    GAME_OVER: 'gameOver'
}

function makeSlot(color) {
    return {
        color: color,
        hammerId: null,
        hatId: null,
        state: SLOT_STATE.UNASSIGNED,
        score: 0
    }
}

const colors = 	['blue', 'orange', 'purple', 'lgtgreen', 'turkiz', 'yellow', 'white', 'pink']

export class Store {
  @observable slots = colors.map(c => makeSlot(c))
  @observable gameState = 'join'
}

