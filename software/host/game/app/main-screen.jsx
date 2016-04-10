'use strict'

import React from 'react';

const fontSize = "150px"

const topContainerStyle = {
    width: "100%",
    display: "flex",
    flexDirection: "column",
    height: "850px",
    backgroundColor: "green"
}

const rowContainerStyle = {
    width: "100%",
    height: "40%",
    display: "flex",
    flexDirection: "row",    
}

const messageAreaStyle = {
    width: "100%",
    paddingTop: "20px",
    paddingBottom: "20px",
    backgroundColor: "red"
}

const slotAreaStyle = {
    width: "25%",
    margin: "40px",
    padding: "20px",
    borderRadius: "30px",
    position: "relative",
    backgroundColor: "gray"
}

const slotDataStyle = {
    position: "absolute",
    fontSize: "150px",
    top: "50%",
    left: "50%",
    transform: "translate(-50%, -50%)"
}

export default class MainScreen extends React.Component {
    constructor(props) {
        super(props)
    }
    
    render() {
        const slot = id => 
            <div key={id} style={slotAreaStyle}>
                <span style={slotDataStyle}>{id}
                </span>
            </div>
            
        return (
            <div style={topContainerStyle}>
                <div style={messageAreaStyle}>
                    This is a message
                </div>
                <div style={rowContainerStyle}>
                    {[slot(0), slot(1), slot(2), slot(3)]}
                </div>
                <div style={rowContainerStyle}>
                    {[slot(4), slot(5), slot(6), slot(7)]}
                </div>
            </div>
        )
        
    }
}