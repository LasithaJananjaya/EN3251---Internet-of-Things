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
* Implementation of the CoapResourceHandler API for the Copper-Chrome extension.
* As it is not possible to install a custom protocol handler, the selected coap resource is appended to the extension URL 
* as the search parameter
*/
Copper.CoapResourceHandler = function(){
};

Copper.CoapResourceHandler.resolveCoapResource = function(callback){
	let search = window.location.search;
    let uri = undefined;
    if (search && search.startsWith("?")){
        uri = Copper.StringUtils.parseUri(decodeURIComponent(search.substr(1)));
    }
    if (uri === undefined){
        Copper.StartupWindowAdapter.openStartupWindow(function(value) {
            uri = Copper.StringUtils.parseUri(value);
            if (uri === undefined){
                Copper.PopupWindowAdapter.openErrorWindow("Enter URL", "Please enter a valid URL", true);
            }
            else {
                Copper.CoapResourceHandler.changeCoapResource(uri.protocol ? uri.protocol : "coap", uri.address, uri.port ? uri.port : Copper.CoapConstants.DEFAULT_PORT, uri.path, uri.query);
            }
        });
    }
    else {
        callback(uri.protocol ? uri.protocol : "coap", uri.address, uri.port ? uri.port : Copper.CoapConstants.DEFAULT_PORT, uri.path, uri.query);
    }
};

Copper.CoapResourceHandler.changeCoapResource = function(protocol, remoteAddress, remotePort, path, query){
	window.location.search = "?" + encodeURIComponent((protocol ? protocol + "://" : "") + remoteAddress + ":" + remotePort +
				(path ? ("/" + path) : "") + (query ? ("?" + query) : ""));
};

Copper.CoapResourceHandler.goToResourceSelection = function(){
    window.location.search = "";
};