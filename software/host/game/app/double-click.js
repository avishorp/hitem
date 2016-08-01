'use strict'

const LONG_CLICK_TIME = 2000
const DOUBLE_CLICK_WINDOW = 500

function ClikDetector(el) {
	this.state = {
		s: 'noclick',
		kcode: null	
	}
	
	// Hook the element events
	el.addEventListener('keydown', e => {
		if (this.state.s === 'noclick')
	})
}