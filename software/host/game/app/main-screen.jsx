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
        const store = this.props.store


        const slot = id => <Slot 
            key={id}
            color={store.slots[id].color}
            state={store.slots[id].state}
            score={store.slots[id].score}
            hitState='none'
            />
            
        let message
        switch(store.gameState) {
            case GAME_STATE.JOIN:
                message = (<div>HIT YOUR OWN HAT TO JOIN THE GAME</div>)
                if (this.props.ready)
                    message = (<div><div>{message}</div><div style={{ fontSize: "80px", marginTop: "20px" }}>HIT START TO PLAY</div></div>)
                break
                
            case GAME_STATE.COUNTDOWN:
                message = (<div>Game starts in {this.props.countdownVal}</div>)
                break
                
            case GAME_STATE.GAME:
                message = (<div style={{ fontSize: "170px" }}>Hit'em!</div>)
                break
                
            case GAME_STATE.GAME_OVER:
                message = (<div style={{ fontSize: "170px" }}>We have a winner</div>)
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

