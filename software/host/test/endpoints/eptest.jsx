import React from 'react';
import ReactDOM from 'react-dom';
import {observer, computed} from "mobx-react";
import {observable} from "mobx"
import autobind from 'autobind'
import config from '../../config.json'
import {server, colors} from '../../epserver'

class Store {
  @observable units = []  

  @autobind
  joinUnit(unit) {
    this.units.push(unit)
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
                setColor={color => this.props.setColor(u.id, color, 70)}/>)}
            </div>)
    }
}

@observer
class EndpointController extends React.Component {
  render() {
      const epid = this.props.epid

    return (<div>
        <span className='unit-id'>ID: {epid}</span>
        <span className='unit-personality'>Type: {this.props.personality}</span>
        {colors.map(c => <a href="#" key={c} onClick={_ => this.props.setColor(c)}> {c}</a>)}
        </div>)
  }
}





window.onload = function() {
    // Start discovery service
	//discovery(config.discovery, config.endpoint.port, console)
	
	// Endpoint server
	const eps = new server(config.endpoint, console)
    const sc = eps.setColor
    eps.on('join', u => store.joinUnit(u))
    
     
  ReactDOM.render(<div>Hello React<EndpointControllers 
        store={store} 
        setColor={(id, color, intensity) => eps.setColor(id, color, intensity)}/>
    </div>,
    document.getElementById("app"));
}
