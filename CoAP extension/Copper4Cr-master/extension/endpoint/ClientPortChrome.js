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
* Implementation of the ClientPort API for chrome
*/
Copper.ClientPort = function(){
};

Copper.ClientPort.connect = function(clientId, finalDisconnectHandler, callback){
	
	// UPDATE with value from chrome://extensions/
	let appId = "ipihibioanajejgfghbbdnjjkflfcbko";
	
	let port = new Copper.Port(chrome.runtime.connect(appId), clientId);

	let connectedCallback = function(){
        if (port !== undefined){
            port.registerDisconnectCallback(finalDisconnectHandler);
        }
        callback(port);
	};
    let firstTimeout = Copper.TimeUtils.setTimeout(connectedCallback, 600);

    port.registerDisconnectCallback(function(){
        // app not started
        Copper.Log.logFine("Starting application");
        Copper.TimeUtils.clearTimeout(firstTimeout);
        let secondTimeout = Copper.TimeUtils.setTimeout(connectedCallback, 750)
        Copper.PopupWindowAdapter.openInfoWindow("Starting...", "Try to start the Copper Application", false);
        chrome.management.launchApp(appId, function(){
            port = new Copper.Port(chrome.runtime.connect(appId), clientId);
            port.registerDisconnectCallback(function(){
                // app was not started
                Copper.TimeUtils.clearTimeout(secondTimeout);
                Copper.PopupWindowAdapter.openErrorWindow("Copper App not installed", "This extension needs the Copper application to send Coap-Messages. Please install the app (<a href=\"https://chrome.google.com/webstore/detail/" + appId + "\">Chrome Web Store</a>) and reload.", false);
            });
        });
    });
};