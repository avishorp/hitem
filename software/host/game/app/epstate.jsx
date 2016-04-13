'use strict'

import util from 'util'
import React from 'react'
import { connect } from 'react-redux'

class EPState extends React.Component {
	constructor() {
		super()
	}
	
	componentWillReceiveProps(nextProps) {
		nextProps.hammers.forEach(v => { 
			const { color, id } = v

			if (id || (id === 0)) {
				const prevColor = this._findColorById(this.props.hammers, id)
				console.log(util.format("color: %s prevcolor: %s id: %d", color, prevColor, id))

				if (color !== prevColor) {
					if (color === 'unassigned')
						this.props.setIndication(id, 'blimp')
					else {
						this.props.setColor(id, color, 70)
					}
				}
			}
				
		})

		nextProps.hats.forEach(v => {
			const { color, id } = v
			
			if (id || (id === 0)) {
				const prevColor = this._findColorById(this.props.hats, id)
			
				if (color !== prevColor) {
					if (color != 'unassigned') {
						if (color === 'chirp')
							this.props.setIndication(id, 'chirp')
						else
							this.props.setColor(id, color, 70)
					}
					else
						this.props.setIndication(id, 'blimp')
				}
			}
		})
	}
	
	_findColorById(coll, id) {
		const i = coll.findIndex(v => v['id']===id)
		return i===-1? null : coll[i].color
	}
	
	render() {
		// This is a worker component, it doesn't render anything meaningful
		return(<div/>)
	}
}

function mapStateToProps(state) {
	return {
		hammers: state.get('slots')
			.map(v => ({ id: v.get('hammerId'), color: v.get('color')}))
			.toJS(),
		hats: state.get('slots')
			.map(v => ({ id: v.get('hatId'), color: v.get('hatColor')}))
			.toJS()
	}
}

const mapPropsToDispatch = () => ({})

export default connect(mapStateToProps, mapPropsToDispatch)(EPState)
