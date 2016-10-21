'use strict'

import React from 'react';
import Slot from './slot'
import { observer, computed } from "mobx-react";


const fontSize = "150px"

const topContainerStyle = {
    width: "100%",
    display: "flex",
    flexDirection: "column",
    height: "1000px"
}

const rowContainerStyle = {
    width: "100%",
    height: "40%",
    display: "flex",
    flexDirection: "row",    
}

const messageAreaStyle = {
    width: "100%",
    height: "350px",
    fontFamily: "vinque",
    fontSize: "180px",
    paddingTop: "20px",
    paddingBottom: "20px",
    textAlign: "center",
    color: "black"
}


@observer
export default class Deathmatch extends React.Component {
    constructor(props) {
        super(props)
    }
    
    render() {
        const slots = this.props.slots

        const slot = id => <Slot 
            key={id}
            color={slots[id].color}
            state={slots[id].state}
            score={slots[id].score}
            hatColor={slots[id].hatColor}
            hammerColor={slots[id].hammerColor}
            hitState={slots[id].hitState}
            />
            
        let message = "testing"
            
        return (
            <div style={topContainerStyle}>
                <div style={messageAreaStyle}>
                    <img src="skull.png" height="200px" style={{ marginRight: '40px' }}/>
                    Deathmatch
                    <img src="skull.png" height="200px" style={{ marginLeft: '40px' }}/>
                </div>
                <div style={rowContainerStyle}>
                    <div style={{ width: '10%' }}></div>
                    {slot(0)}
                    <div style={{ textAlign: 'center', paddingTop: '120px', width: '20%'}}><img src="vs.png"/></div>
                    {slot(1)}
                    <div style={{ width: '10%' }}></div>
                </div>
            </div>
        )
        
    }
}

