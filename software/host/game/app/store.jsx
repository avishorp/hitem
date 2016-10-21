import { observer } from "mobx-react";
import { observable, computed } from "mobx"
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

function makeSlot(color, index) {
    return {
        color: color,
        hammerId: null,
        hatId: null,
        state: SLOT_STATE.UNASSIGNED,
        score: 0,
        hatColor: 'chirp',
        hammerColor: 'chirp',
        index: index,
        hitState: 'none',
        indication: null
    }
}

const colors = 	['blue', 'orange', 'purple', 'turkiz', 'yellow', 'white', 'pink', 'lgtgreen']


export class Store {
  @observable slots = colors.map((c, index) => makeSlot(c, index))
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
      if (this.gameState === GAME_STATE.GAME)
        this._actionHitGame(e)
      if(this.gameState === GAME_STATE.DEATHMATCH)
        this._actionHitDeathmatch(e)
  }

  _actionHitJoin(e) {
      // If there are more than 8 hits - ignore them
      if (this.currentSlot <= 7) {
          // Check that the hammer or the hat are not already assigned
          const assignedHats = this.slots.filter(s => s.hatId).map(s => s.hatId)
          const assignedHammers = this.slots.filter(s => s.hammerId).map(s => s.hammerId)
          if ((assignedHats.indexOf(e.hatId) === -1) && (assignedHammers.indexOf(e.hammerId) === -1)) {
              // Do the assignment
              let s = this.slots[this.currentSlot]
              s.hammerId = e.hammerId
              s.hatId = e.hatId
              s.hatColor = s.color
              s.hammerColor = s.color
              s.state = SLOT_STATE.ASSIGNED
              this.currentSlot += 1
          }
      }
  }

  _actionHitGame(e) {
      const checkGameOver = slot => {
          if (slot.score <= 0) {
            slot.state = SLOT_STATE.GAME_OVER
            slot.hatColor = 'none'
            slot.hammerColor = 'red'
            slot.hitState = 'none'

            // Schedule hammer light turn off
            setTimeout(() => {
                if (slot.state === SLOT_STATE.GAME_OVER)
                    slot.hammerColor = 'none'
            }, 2000)
            return true
          }
          else
            return false
      }

      const { hatId, hammerId } = e

      // If wer'e in transition, ignore the hit
      if (this.gameSubState === GAME_SUB_STATE.WAIT_HIT) {
          // Find the color of the hammer
          let hammerSlot = this.slots.find(slot => slot.hammerId === hammerId)
          if (!hammerSlot) {
              console.log("Unknon hammer " + hammerId)
              return
          }
          if (hammerSlot.state != SLOT_STATE.ACTIVE)
            // Ignore hit by non-active hammer
            return

          let hatSlot = this.slots.find(slot => slot.hatId === hatId)
          if (!hatSlot) {
              console.log("Unknon hat " + hammerId)
              return              
          }
          if (hatSlot.state != SLOT_STATE.ACTIVE)
            // Ignore hit on non-active hat
            return

          // If the colors of the hammer's slot is identical to the hat color
          // of the hat slot - it's a positive hit
          if (hammerSlot.color === hatSlot.hatColor) {
              // Add one point to the hammer
              hammerSlot.score += 1

              // Decrement one point from the hat
              hatSlot.score -= 1
              checkGameOver(hatSlot)

              // Indicate the hit
              this._setHammerIndication(hammerSlot, 'green')
              this._setHammerIndication(hatSlot, 'red')

              // Reshuffle
              this.actionGameShuffle()
          }
          else {
              // Negative hit
              hammerSlot.score -= this.config.penalty

              // Indicate the hit
              this._setHammerIndication(hammerSlot, 'red')

              if (checkGameOver(hammerSlot))
                this.actionGameShuffle()
          }

          // Check for "deathmatch" condition
          const deathmatchSlots = this.deatchmatchSlots
          if (this.deatchmatchSlots) {
              // Set the game state
              this.gameState = GAME_STATE.DEATHMATCH

              // Assign colors
              deathmatchSlots[0].hatColor = deathmatchSlots[1].color
              deathmatchSlots[1].hatColor = deathmatchSlots[0].color

          }
      }
  }

  _actionHitDeathmatch(e) {
      const slots = this.deatchmatchSlots
      assert(slots)

      const { hatId, hammerId } = e

      console.log(slots[0].hammerId)
      console.log(slots[0].hatId)
      console.log(slots[1].hammerId)
      console.log(slots[1].hatId)

      console.log(e)
      // Check which one was hit
      if ((slots[0].hatId === hatId) && (slots[1].hammerId === hammerId)) {
          console.log("A")
          slots[0].score -= 1
      }
      else if ((slots[0].hammerId = hammerId) && (slots[1].hatId === hatId)) {
          console.log("B")
          slots[1].score -= 1
      }

      // Check game over condition
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
      this.slots.forEach(slot => { if (slot.state === SLOT_STATE.ACTIVE) slot.hatColor = 'chirp' })

      // Schedule transition to WAIT_HIT state
      setTimeout(() => {
          // Switch to WAIT_HIT
          this.gameSubState = GAME_SUB_STATE.WAIT_HIT

          // Shuffle colors for all hats
          const activeHammer = this.slots.filter(slot => slot.state === SLOT_STATE.ACTIVE)
          const activeHammerColor = activeHammer.map(slot => slot.color)
          const activeHammerIndex = activeHammer.map(slot => slot.index)

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

  _setHammerIndication(slot, indication) {
      // If there's a current pending indication, clean it
      if (slot.indication)
        clearTimeout(slot.indication)

      // Set the indication
      assert((indication === 'green') || (indication === 'red'))
      slot.hammerColor = indication + "_pulse"
      slot.hitState = indication

      // Schedule indication clear
      setTimeout(() => {
          // If the slot state is not active, ignore
          if (slot.state === SLOT_STATE.ACTIVE) {
              slot.hammerColor = slot.color
              slot.hitState = 'none'
          }
      }, 1500)
  }

  // In case of a "deathmatch", this will return the two active slots
  @computed get deatchmatchSlots() {
      const active = this.slots.filter(s => s.state === SLOT_STATE.ACTIVE)
      if (active.length !== 2)
        // This should not happen
        return null
      else
        return active
  }
}

