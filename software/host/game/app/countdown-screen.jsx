
import React from 'react'
import { observer } from "mobx-react";

@observer
export default class CountdownScreen extends React.Component {
    constructor(props) {
        super(props)
    }

    render() {
        return <div>{this.props.value}</div>
    }    
}