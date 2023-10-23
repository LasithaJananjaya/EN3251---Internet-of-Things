package protocol_pros_server;

import org.eclipse.californium.core.CoapServer;

public class TestServer extends CoapServer {
    public static void main(String[] args) {
        TestServer ts = new TestServer();
        ProtocolProsResource hello = new ProtocolProsResource("hello-world");
        ts.add(hello);
        ts.start();
    }
}