import { observer, computed } from "mobx-react";
import { observable } from "mobx"
import autobind from 'autobind'
import assert from 'assert'
import { knuthShuffle } from 'knuth-shuffle'


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

export const GAME_SUB_STATE = {
    SHUFFLE: 'shuffle',
    WAIT_HIT: 'waitHit'
}

function makeSlot(color) {
    return {
        color: color,
        hammerId: null,
        hatId: null,
        state: SLOT_STATE.UNASSIGNED,
        score: 0,
        hatColor: null
    }
}

const colors = 	['blue', 'orange', 'purple', 'lgtgreen', 'turkiz', 'yellow', 'white', 'pink']


export class Store {
  @observable slots = colors.map(c => makeSlot(c))
  @observable gameState = GAME_STATE.JOIN
  @observable gameSubState = GAME_SUB_STATE.SHUFFLE
  @observable countdownValue = null
  currentSlot = 0

  constructor(config) {
      this.config = config
  }

  actionHit(e) {
      if (this.gameState === GAME_STATE.JOIN)
        this._actionHitJoin(e)
  }

  _actionHitJoin(e) {
      // If there are more than 8 hits - that's a problem
      assert(this.currentSlot <= 7)

      let s = this.slots[this.currentSlot]
      s.hammerId = e.hammerId
      s.hatId = e.hatId
      s.hatColor = s.color
      s.state = SLOT_STATE.ASSIGNED
      this.currentSlot += 1
  }

  actionCountdown() {
      console.log('countdown')
      // Set the initial countdown value and set the game mode to COUNTDOWN
      this.countdownValue = 3
      this.gameState = GAME_STATE.COUNTDOWN

      // Decrement countdown after constant time
      const COUNTDOWN_DELAY = 1000
      setTimeout(() => { this.countdownValue = 2 }, COUNTDOWN_DELAY)
      setTimeout(() => { this.countdownValue = 1 }, COUNTDOWN_DELAY*2)
      setTimeout(() => { this.countdownValue = 0 }, COUNTDOWN_DELAY*3)
      setTimeout(() => { this.actionStartGame() }, COUNTDOWN_DELAY*4)

  }

  actionStartGame() {
      // Set game mode
      this.gameState = GAME_STATE.GAME

      // Initialize slots
      this.slots.forEach(slot => {
          if (slot.state === SLOT_STATE.ASSIGNED) {
              // Turn all assigned slots to active and set initial score
              slot.state = SLOT_STATE.ACTIVE
              slot.score = this.config.startingScore
          }
          else {
              // Unassigned slots turn inactive
              slot.state = SLOT_STATE.INACTIVE
          }
        })

      // Swith to shuffle mode to actually start the game
      this.actionGameShuffle()
  }

  actionGameShuffle() {
      this.gameSubState = GAME_SUB_STATE.SHUFFLE

      // Set all hat colors to 'chirp'
      this.slots.forEach(slot => slot.hatColor = 'chirp')

      // Schedule transition to WAIT_HIT state
      setTimeout(() => {
          // Switch to WAIT_HIT
          this.gameSubState = GAME_SUB_STATE.WAIT_HIT

          // Shuffle colors for all hats
          const activeHammer = this.slots.filter(slot => slot.state === SLOT_STATE.ACTIVE)
          const activeHammerColor = activeHammer.map(slot => slot.color)
          const activeHammerIndex = activeHammer.map((slot, index) => index)

          // This function checks whether in two arrays, a and b, no same index has the same value 
          const checkNonIdentity = (a, b) =>
            a.reduce((prev, value, index) => prev & (value !== b[index]), true)

          // Shuffle until the result is legal (no same hat & hammer has the same color)
          let hatColors = activeHammerColor.slice()
          while (!checkNonIdentity(activeHammerColor, hatColors)) {
            hatColors = knuthShuffle(hatColors)
          }

          // Assign the shuffled colors back to the slots
          activeHammerIndex.forEach((slotIndex, colorIndex) => this.slots[slotIndex].hatColor = hatColors[colorIndex])
      }, this.config.shufflePeriod)

  }

  actionStartButton() {
      console.log('start')
      if ((this.gameState === GAME_STATE.JOIN) && (this.currentSlot >= this.config.minPlayers))
        this.actionCountdown()
  }

  actionStopButton() {

  }
}

