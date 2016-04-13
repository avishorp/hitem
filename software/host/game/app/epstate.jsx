'use strict'

import React from 'react'
import { connect } from 'react-redux'

class EPState extends React.Component {
	constructor() {
		super()
	}
	
	componentWillReceiveProps(nextProps) {
		nextProps.hammers.forEach(v => { 
			const { color, id } = v
			
			const prevColor = this._findColorById(props.hammers, id)
			if (color !== prevColor) {
				if (color === 'unassigned')
					this.props.setIndication(id, 'blimp')
				else {
					this.props.setColor(id, color, 70)
				}
			}
				
		})
			
		nextProps.hats.forEach(v => {
			const { color, id } = v.color
			const prevColor = this._findColorById(props.hats, id)
			
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
		})
	}
	
	_findColorById(coll, id) {
		const i = coll.findIndex(v => v.get(id)===id)
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
			.filter(v => (v.get('color') !== 'unassigned'))
			.map(v => ({ id: v.get('hammerId'), color: v.get('color')}))
			.toJS(),
		hats: state.get('hatColor')
			.map((v, i) => ({ id: i, color: v}))
			.toJS()
	}
}

const mapPropsToDispatch = () => ({})

export default connect(mapStateToProps, mapPropsToDispatch)(EPState)
