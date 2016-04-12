'use strict'

import React from 'react';
import { connect } from 'react-redux' 
import Slot from './slot'

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
    height: "200px",
    fontFamily: "lcd",
    fontSize: "80px",
    paddingTop: "20px",
    paddingBottom: "20px",
    textAlign: "center",
    backgroundColor: "Chocolate",
    color: "white"
}



class JoinPlayScreen extends React.Component {
    constructor(props) {
        super(props)
    }
    
    render() {
        const slot = id => <Slot 
            key={id}
            color={this.props.slots[id].color}
            score={this.props.slots[id].score}
            mode={this.props.major}
            />
            
        return (
            <div style={topContainerStyle}>
                <div style={messageAreaStyle}>
                    <div>HIT YOUR OWN HAT TO JOIN THE GAME</div>
                    { this.props.ready? <div style={{ fontSize: "50px", marginTop: "20px" }}>HIT START TO PLAY</div> : <div/>}
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

function mapStateToProps(state) {
    return {
        major: state.getIn(['global', 'major']),
        slots: state.getIn(['stateJoin', 'slots']).toJS(),
        ready: state.getIn(['stateJoin', 'ready'])
    }
}

function mapDispatchToProps(dispatch) {
    return {}
}

export default connect(mapStateToProps, mapDispatchToProps)(JoinPlayScreen)
