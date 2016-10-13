'use strict'

import React from 'react';
import Slot from './slot'
import { observer, computed } from "mobx-react";
import { GAME_STATE } from './store'


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
    height: "200px",
    fontFamily: "lcd",
    fontSize: "120px",
    paddingTop: "20px",
    paddingBottom: "20px",
    textAlign: "center",
    backgroundColor: "Chocolate",
    color: "white"
}


@observer
export default class MainScreen extends React.Component {
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
            hitState='none'
            />
            
        let message
        switch(this.props.mode) {
            case 'join':
                message = (<div>HIT YOUR OWN HAT TO JOIN THE GAME</div>)
                if (this.props.ready)
                    message = (<div><div>{message}</div><div style={{ fontSize: "80px", marginTop: "20px" }}>HIT START TO PLAY</div></div>)
                break
                              
            case 'game':
                message = (<div style={{ fontSize: "170px" }}>Hit'em!</div>)
                break
                
        }
            
        return (
            <div style={topContainerStyle}>
                <div style={messageAreaStyle}>
                    { message }
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

