'use strict'

import React from 'react'

const colorTranslateTable = {
	'blue': 'Blue',
	'orange': 'DarkOrange',
	'purple': 'DarkOrchid',
	'lgtgreen': 'LightGreen',
	'turkiz': 'Turquoise',
	'yellow': 'Yellow',
	'white': 'White',
	'pink': 'HotPink'
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
        let computedAreaStyle;
        let computedDataStyle;
        let content;

        switch(this.props.state) {
            case 'unassigned':
                computedAreaStyle = slotAreaStyle
                content = ''
                break
                
            case 'assigned':
                computedAreaStyle = Object.assign({}, slotAreaStyle, 
                    { backgroundColor: colorTranslateTable[this.props.color] })
                content = <img src="checkmark.svg" width="200"/>
                break
                
            case 'active':
                // Game mode, assigned color
                computedAreaStyle = Object.assign({}, slotAreaStyle, 
                    { backgroundColor: colorTranslateTable[this.props.color] })
                content = this.props.score
                break                

            case 'inactive':
                computedAreaStyle = Object.assign({}, slotAreaStyle, 
                    { opacity: 0 })
                break                

            case 'gameOver':                            
                computedAreaStyle = Object.assign({}, slotAreaStyle, 
                    { backgroundColor: colorTranslateTable[this.props.color] })
                content = <img src="game_over.svg" width="400"/>
                break
                
            case 'winner':
                computedAreaStyle = Object.assign({}, slotAreaStyle, 
                    { backgroundColor: colorTranslateTable[this.props.color] })
                content = <img src="cup.svg" width="200"/>                
        }
        
        // If the slot is "hit", mark it with a border`
        if (this.props.hitState !== 'none')
                computedAreaStyle.border = "8px solid " + this.props.hitState
             
        
        return (
            <div style={computedAreaStyle}>
                <span style={slotDataStyle}>{content}</span>
            </div>
        )
    }

}