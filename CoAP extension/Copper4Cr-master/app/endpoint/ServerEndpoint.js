/*******************************************************************************
 * Copyright (c) 2016, Institute for Pervasive Computing, ETH Zurich.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 * 
 * This file is part of the Copper (Cu) CoAP user-agent.
 ******************************************************************************/
 
/* 
*  Server Endpoint for one given client (e.g. extension or page)
*
*  The following state transitions are legal
*
*     STATE_CONNECTED            (Register)
*          |  Λ
* register |  | unregister
*          V  |
*  STATE_UDP_SOCKET_READY        (Send, Receive, Unregister, Disconnect)
*             |
* disconnect  |
*             V
*   STATE_DISCONNECTED
*/
Copper.ServerEndpoint = function(port, id){
	if (!port || !Number.isInteger(id)){
		throw new Error("Illegal Arguments");
	}
	this.id = id;
	this.port = port;
	
	let thisRef = this;

	this.port.registerDisconnectCallback(function(){
		thisRef.handleClientDisconnected();
	});

	this.eventCallback = function(event){
		return thisRef.dispatchEvent(event);
	};
	Copper.Event.registerCallback(this.eventCallback, this.id);

	this.state = Copper.ServerEndpoint.STATE_CONNECTED;
	Copper.Log.logFine("Server Endpoint " + this.id + " created");
};

/* State constants */
Copper.ServerEndpoint.STATE_CONNECTED = 0;
Copper.ServerEndpoint.STATE_UDP_SOCKET_READY = 1;
Copper.ServerEndpoint.STATE_DISCONNECTED = 2;

/* prototype */
Copper.ServerEndpoint.prototype.port = undefined;
Copper.ServerEndpoint.prototype.id = undefined;
Copper.ServerEndpoint.prototype.state = undefined;
Copper.ServerEndpoint.prototype.transmissionHandler = undefined;
Copper.ServerEndpoint.prototype.eventCallback = undefined;
Copper.ServerEndpoint.prototype.currentRequest = undefined;


/* Callbacks */
/* Callback for the Copper event queue */
Copper.ServerEndpoint.prototype.dispatchEvent = function(event){
	if (!Number.isInteger(event.type)){
		throw new Error("Illegal Arguments");
	}
	try {
		switch(event.type){
			case Copper.Event.TYPE_ERROR_ON_SERVER:
				return this.onError(event.data.errorType, event.data.errorMessage, event.data.endpointReady);

			case Copper.Event.TYPE_REGISTER_CLIENT:
				return this.onRegisterClient(event.data.remoteAddress, event.data.remotePort, event.data.settings);
			case Copper.Event.TYPE_CLIENT_REGISTERED:
				this.state = Copper.ServerEndpoint.STATE_UDP_SOCKET_READY;
				this.port.sendMessage(event);
				return true;
			case Copper.Event.TYPE_UNREGISTER_CLIENT:
				return this.onUnregisterClient();
			case Copper.Event.TYPE_UPDATE_SETTINGS:
				return this.onUpdateSettings(event.data.settings);

			case Copper.Event.TYPE_SEND_COAP_MESSAGE:
				return this.onClientSendCoapMessage(event.data.coapMessage, event.data.blockwiseEnabled);
			case Copper.Event.TYPE_COAP_MESSAGE_SENT:
			case Copper.Event.TYPE_MESSAGE_TRANSMISSION_TIMED_OUT:
			case Copper.Event.TYPE_MESSAGE_TRANSMISSION_CONFIRMED:
			case Copper.Event.TYPE_MESSAGE_TRANSMISSION_COMPLETED:
				this.port.sendMessage(event);
				return true;

			case Copper.Event.TYPE_COAP_MESSAGE_RECEIVED:
			case Copper.Event.TYPE_UNKNOWN_COAP_MESSAGE_RECEIVED:
			case Copper.Event.TYPE_DUPLICATE_COAP_MESSAGE_RECEIVED:
			case Copper.Event.TYPE_RECEIVED_PARSE_ERROR:
				this.port.sendMessage(event);
				return true;
			

			case Copper.Event.TYPE_CANCEL_REQUESTS:
				return this.onCancelRequests();
   			
			case Copper.Event.TYPE_REQUEST_COMPLETED:
			case Copper.Event.TYPE_OBSERVE_REQUEST_FRESH:
            case Copper.Event.TYPE_OBSERVE_REQUEST_OUT_OF_ORDER:
			case Copper.Event.TYPE_REQUEST_RECEIVE_ERROR:
			case Copper.Event.TYPE_REQUEST_TIMEOUT:
			case Copper.Event.TYPE_REQUEST_CANCELED:
				this.port.sendMessage(event);
				return true;

			default:
				Copper.Log.logWarning("Unknown event type " + event.type);
				return false;
		}
	} catch (exception) {
		Copper.Log.logError("Error on endpoint " + this.id + ": " + exception.stack);
		return this.onError(Copper.Event.ERROR_GENERAL, "Endpoint Error: " + exception.message, false);
	}
};

/* Callback for the server port (called when the client disconnects) */
Copper.ServerEndpoint.prototype.handleClientDisconnected = function(){
	if (this.state === Copper.ServerEndpoint.STATE_UDP_SOCKET_READY){
		this.transmissionHandler.close();
		this.transmissionHandler = undefined;
	}
	if (this.state !== Copper.ServerEndpoint.STATE_DISCONNECTED){
		this.state = Copper.ServerEndpoint.STATE_DISCONNECTED;
		Copper.Event.removeEventsForEndpoint(this.id);
		Copper.Event.unregisterCallback(this.eventCallback, this.id);
		Copper.Log.logFine("Server Endpoint " + this.id + " closed");
	}
};

/* Implementation of the different events */
Copper.ServerEndpoint.prototype.onError = function(errorType, errorMessage, endpointReady){
	if (this.state === Copper.ServerEndpoint.STATE_UDP_SOCKET_READY && !endpointReady){
		this.transmissionHandler.close();
		this.transmissionHandler = undefined;
		this.state = Copper.ServerEndpoint.STATE_CONNECTED;
	}
	this.port.sendMessage(Copper.Event.createErrorOnServerEvent(errorType, errorMessage, endpointReady, this.id));
	return true;
};

Copper.ServerEndpoint.prototype.onRegisterClient = function(remoteAddress, remotePort, settings){
	if (this.state !== Copper.ServerEndpoint.STATE_CONNECTED){
		this.onError(Copper.Event.ERROR_ILLEGAL_STATE, "Illegal State", this.state === Copper.ServerEndpoint.STATE_UDP_SOCKET_READY);
	}
	else {
		this.transmissionHandler = new Copper.TransmissionHandler(new Copper.UdpClient(), remoteAddress, remotePort, settings, this.id);
		this.transmissionHandler.bind();
	}
	return true;
};

Copper.ServerEndpoint.prototype.onUnregisterClient = function(){
	if (this.state !== Copper.ServerEndpoint.STATE_UDP_SOCKET_READY){
		this.onError(Copper.Event.ERROR_ILLEGAL_STATE, "Illegal State", false);
	}
	else {
		this.transmissionHandler.close();
		this.transmissionHandler = undefined;
		this.state = Copper.ServerEndpoint.STATE_CONNECTED;
		Copper.Log.logFine("Server Endpoint " + this.id + ": Client unregistered");
	}
	return true;
};

Copper.ServerEndpoint.prototype.onUpdateSettings = function(settings){
	if (this.transmissionHandler !== undefined){
		this.transmissionHandler.updateSettings(settings);
	}
	return true;
};

Copper.ServerEndpoint.prototype.onCancelRequests = function(){
	if (this.currentRequest !== undefined){
		this.currentRequest.cancel();
		this.currentRequest = undefined;
	}
	return true;
};

Copper.ServerEndpoint.prototype.onClientSendCoapMessage = function(coapMessage, blockwiseEnabled){
	if (this.state !== Copper.ServerEndpoint.STATE_UDP_SOCKET_READY){
		this.onError(Copper.Event.ERROR_ILLEGAL_STATE, "Illegal State", false);
	}
	else {
		this.onCancelRequests();
		this.currentRequest = new Copper.SingleRequestHandler(coapMessage, blockwiseEnabled, this.transmissionHandler, this.transmissionHandler.settings, this.id);
		this.currentRequest.start();
	}
	return true;
};