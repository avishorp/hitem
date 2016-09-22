import React from 'react';
import ReactDOM from 'react-dom';
import {observer, computed} from "mobx-react";
import {observable} from "mobx"
import autobind from 'autobind'
import config from '../../config.json'
import EPServer  from '../../epserver'

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
            {units.map(u => <EndpointController key={u.id} epid={u.id} personality={u.personality}/>)}
            </div>)
    }
}

@observer
class EndpointController extends React.Component {
  render() {
    return (<div>
        <span className='unit-id'>ID: {this.props.epid}</span>
        <span className='unit-personality'>Type: {this.props.personality}</span>
        </div>)
  }
}





window.onload = function() {
    // Start discovery service
	//discovery(config.discovery, config.endpoint.port, console)
	
	// Endpoint server
	const eps = new EPServer(config.endpoint, console)
    eps.on('join', u => store.joinUnit(u))
    
     
  ReactDOM.render(<div>Hello React<EndpointControllers store={store}/></div>,
    document.getElementById("app"));
}
