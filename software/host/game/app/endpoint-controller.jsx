import React from 'react'
import { Store, GAME_STATE } from './store'
import { observer } from "mobx-react";


@observer
export default class EndpointController extends React.Component {
    render() {
        const slots = this.props.slots
        const setColor = this.props.setColor

        // For each slot, set the color of the hat and hammer associated with it
        slots.forEach(slot => {
            if (slot.hatId)
                setColor(slot.hatId, slot.hatColor, 70)

            if (slot.hammerId)
                setColor(slot.hammerId, slot.hammerColor, 70)
        })

        // This component does not render any actual DOM, it
        // returns an empty DIV to become a valid React component
        return <div/>
    }
}