import EventEmitter from 'events'
import { knuthShuffle } from 'knuth-shuffle'

const selectRandom = a => a[Math.floor(Math.random()*a.length)]
const NUM_HITS = 50

export default class HitEmulator extends EventEmitter {
    constructor() {
        super()
        
        // Create a list of hammer and hat IDs
        const ids = knuthShuffle([1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16])
        this.hats = ids.splice(ids.length/2)
        this.hammers = ids
    }

    run() {
        console.log("Emulation: Starting")
        this.emulateJoinHits(() => {
            console.log("Emulation: Join hits done")
            this.emulateGameHits(() => {
                console.log("Emulation: Game hits done")
            })
        })
    }

    emulateJoinHits(done) {
        const doEmit = i => () => {
            if (i > this.hats.length-1)
                done()
            else {
                this.emit('hit', {
                    hatId: this.hats[i],
                    hammerId: this.hammers[i]
                })
                setTimeout(doEmit(i+1), 1200)
            }
        }
        
        setTimeout(doEmit(0), 2000)
    }

    emulateGameHits(done) {
        const doEmit = i => () => {
            if (i > 0)
                done()

            else {
                // Select a hat+hammer pair randomally
                const hatId = selectRandom(this.hats)
                const hammerId = selectRandom(this.hammerId)

                // Emit a hit event
                this.emit('hit', {
                    hatId: this.hats[i],
                    hammerId: this.hammers[i]
                })

                // Schedule next event
                const nextEventTime = Math.floor(Math.random()*2000) + 200
                setTimeout(doEmit(i-1), nextEventTime)

            }
        }

        doEmit(NUM_HITS)
    }

}
