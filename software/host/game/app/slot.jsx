'use strict'

import React from 'react'

const slotAreaStyle = {
    width: "25%",
    margin: "40px",
    padding: "20px",
    borderRadius: "30px",
    position: "relative",
    backgroundColor: "gray"
}

const slotDataStyle = {
    fontFamily: "digital",
    position: "absolute",
    fontSize: "150px",
    top: "50%",
    left: "50%",
    transform: "translate(-50%, -50%)"
}

export default class Slot extends React.Component {
    constructor(props) {
        super(props)
    }
    
    render() {
        return (
            <div style={slotAreaStyle}>
                <span style={slotDataStyle}>{this.props.score}
                </span>
            </div>
        )
    }

}