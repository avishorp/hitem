'use strict'

import React from 'react'
import { connect } from 'react-redux'

class EPState extends React.Component {
	constructor() {
		super()
	}
	
	componentWillReceiveProps(nextProps) {
		if (this.props) {
			console.log(nextProps)
			nextProps.hammers.forEach(v => { this.props.setColor(v.hammerId, v.color, 70) })
		}
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
			.map(v => ({ hammerId: v.get('hammerId'), color: v.get('color')}))
			.toJS()
	}
}

const mapPropsToDispatch = () => ({})

export default connect(mapStateToProps, mapPropsToDispatch)(EPState)
