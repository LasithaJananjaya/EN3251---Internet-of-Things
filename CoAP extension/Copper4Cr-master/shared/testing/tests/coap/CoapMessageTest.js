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
 
QUnit.test("CoapMessage: Object", function(assert) {
	let conType = new Copper.CoapMessage.Type(0, "CON");
	let getCode = new Copper.CoapMessage.Code(1, "GET");
	let mid = 0x342;
	let token = Copper.ByteUtils.convertToByteArray("token");
	let payload = new ArrayBuffer(100);

	let msg = new Copper.CoapMessage(conType, getCode);
	msg.setMid(mid).setToken(token).setPayload(payload);
	
	assert.deepEqual(msg.type, conType);
	assert.deepEqual(msg.code, getCode);
	assert.deepEqual(msg.mid, mid);
	assert.deepEqual(msg.token, token);
	assert.deepEqual(msg.payload, payload);

	assert.throws(function(){
		msg.setMid(0x1FFFF);
	});
	assert.throws(function(){
		msg.setToken(new ArrayBuffer(10));
	});


	let uriOptionHeader = new Copper.CoapMessage.OptionHeader(3, "Uri-Host", Copper.CoapMessage.OptionHeader.TYPE_STRING, 1, 255, false);
	let etagHeader = new Copper.CoapMessage.OptionHeader(4, "Etag", Copper.CoapMessage.OptionHeader.TYPE_OPAQUE, 1, 8, true);
	let maxAgeHeader = new Copper.CoapMessage.OptionHeader(14, "Max-Age", Copper.CoapMessage.OptionHeader.TYPE_UINT, 0, 4, false, 60);

	assert.deepEqual(msg.getOption(uriOptionHeader), []);
	assert.deepEqual(msg.getOption(etagHeader), []);
	assert.deepEqual(msg.getOption(maxAgeHeader), [60]); // default value

	msg.addOption(etagHeader, "0x4444");
	assert.deepEqual(msg.getOption(etagHeader), ["0x4444"]);

	msg.addOption(etagHeader, "0xF3F2F1F0");
	assert.deepEqual(msg.getOption(etagHeader), ["0x4444", "0xF3F2F1F0"]);

	msg.addOption(uriOptionHeader, "vs0.inf.ethz.ch").addOption(uriOptionHeader, "vs1.inf.ethz.ch", true);
	assert.deepEqual(msg.getOption(uriOptionHeader), ["vs1.inf.ethz.ch"]);

	assert.deepEqual(msg.getOptions(), [msg.options[uriOptionHeader.number], msg.options[etagHeader.number]]);
	assert.ok(msg.toString());

	// Empty message must not contain payload
	assert.throws(function(){
		new Copper.CoapMessage(conType, Copper.CoapMessage.Code.EMPTY).addOption(etagHeader, "0x33");
	});
	assert.throws(function(){
		new Copper.CoapMessage(conType, Copper.CoapMessage.Code.EMPTY).setPayload(payload);
	});
});

QUnit.test("CoapMessage: clone", function(assert) {
	Copper.TestUtils.applyTestsOnDifferentCoapMessages([function(msg){
		Copper.TestUtils.checkCoapMessageEquality(assert, msg.clone(), msg);
	}]);
});