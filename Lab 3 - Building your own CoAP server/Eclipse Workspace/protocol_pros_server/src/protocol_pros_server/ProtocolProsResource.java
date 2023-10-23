package protocol_pros_server;

import org.eclipse.californium.core.CoapResource;
import org.eclipse.californium.core.server.resources.CoapExchange;

public class ProtocolProsResource extends CoapResource {
	public String content = "Hello World";

	public ProtocolProsResource(String name) {
		super(name);
	}

	@Override
	public void handleGET(CoapExchange exchange) {
		exchange.respond(content);
	}

	@Override
	public void handlePUT(CoapExchange exchange) {
		byte[] payload = exchange.getRequestPayload();
		try {
			content = new String(payload, "UTF-8");
			exchange.respond(content);
		} catch (Exception e) {
			e.printStackTrace();
			exchange.respond("Invalid String");
		}
	}
}
