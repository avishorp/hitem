import EventEmitter from 'events'
import { knuthShuffle } from 'knuth-shuffle'


export default class HitEmulator extends EventEmitter {
    constructor() {
        super()
        
        // Create a list of hammer and hat IDs
        const ids = knuthShuffle([1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16])
        this.hats = ids.splice(ids.length/2)
        this.hammers = ids
    }

    run() {
        this.emulateJoinHits(() => {console.log("Emulation done")})
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
}
