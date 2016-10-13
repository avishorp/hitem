
import React from 'react'
import { observer } from "mobx-react";
import assert from 'assert'


const images = {
    0: 'hitem.png',
    1: 'countdown_1.png',
    2: 'countdown_2.png',
    3: 'countdown_3.png',
}

@observer
export default class CountdownScreen extends React.Component {
    constructor(props) {
        super(props)
    }

    render() {
        const img = images[this.props.value]
        assert(img)

        return <img style={{
            margin: '200px auto',
            display: 'block'
        }} 
        src = { img } />
    }    
}