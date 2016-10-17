import React from 'react';
import ReactDOM from 'react-dom';
import {observer, computed} from "mobx-react";
import {observable} from "mobx"
import autobind from 'autobind'
import config from '../../config.json'
import {server, colors} from '../../epserver'
import Slider from 'rc-slider';



class Store {
  @observable units = []  

  @autobind
  joinUnit(unit) {
    this.units.push(unit)
  }

  @autobind
  leaveUnit(uid) {
      const ids = this.units.map(u => u.id)
      const n = ids.indexOf(uid)
      if (n === -1) {
          console.error("Requested removal of non-existing unit")
          return
      }

      this.units = this.units.slice(0, n).concat(this.units.slice(n+1))
  }
}

let store = new Store()

@observer
class EndpointControllers extends React.Component {
    render() {
        const store = this.props.store
        const units = store.units
 
        return (<div>
            {units.map(u => <EndpointController 
                key={u.id} 
                epid={u.id} 
                personality={u.personality}
                setColor={color => this.props.setColor(u.id, color, 70)}
                setThreshold={thresh => this.props.setThreshold(u.id, thresh)}
                />)}

            </div>)
    }
}

@observer
class EndpointController extends React.Component {
    constructor(props) {
        super(props)

        this.state = { hitThreshold: 2400 }
    }

    @autobind
    onThresholdSliderChange(value) {
      this.setState({ hitThreshold: value })
    }

    @autobind
    onThresholdAfterChange(value) {
        console.log(`New threshold value for unit %d: %d`, this.props.epid, value)
        this.props.setThreshold(value)
    }


  render() {
      const epid = this.props.epid

    return (<div style={{ bottomMargin: '15px' }}>
        <span className='unit-id'>ID: {epid}</span>
        <span className='unit-personality'>Type: {this.props.personality}</span>
        {colors.map(c => <a href="#" key={c} onClick={_ => this.props.setColor(c)}> {c}</a>)}
        <span style={{width: '150px', background: 'red'}}>
        <Slider 
            min={2000} 
            max={3000} 
            value={this.state.hitThreshold} 
            style={{width: '50px', display: 'inline'}}
            onChange={this.onThresholdSliderChange}
            onAfterChange={this.onThresholdAfterChange}
            /></span>
        </div>)
  }
}
//




window.onload = function() {
    // Start discovery service
	//discovery(config.discovery, config.endpoint.port, console)
	
	// Endpoint server
	const eps = new server(config.endpoint, console)
    const sc = eps.setColor
    eps.on('join', u => store.joinUnit(u))
    eps.on('leave', u => store.leaveUnit(u))
    
     
  ReactDOM.render(<div><h1>Hit'em Test Application</h1><EndpointControllers 
        store={store} 
        setColor={(id, color, intensity) => eps.setColor(id, color, intensity)}
        setThreshold={(id, threshold) => eps.setThreshold(id, threshold)}
        />
    </div>,
    document.getElementById("app"));
}
