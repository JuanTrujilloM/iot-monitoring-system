import java.io.*;
import java.net.*;

/**
 * ServerConnection.java - Gestión de la conexión TCP con el servidor
 *
 * Responsabilidades:
 *   - Resolver el hostname por DNS (InetAddress.getByName) — nunca IP hardcodeada
 *   - Establecer y mantener la conexión TCP
 *   - Enviar mensajes del protocolo al servidor
 *   - Leer respuestas y alertas en un hilo separado
 *   - Notificar a listeners cuando llega un mensaje (patrón Observer)
 *   - Reconectarse automáticamente si se pierde la conexión
 */
public class ServerConnection {

    public interface MessageListener {
        void onMessage(String message);
        void onDisconnected();
    }

    // TODO: implementar connect(String host, int port)
    // TODO: implementar sendMessage(String msg)
    // TODO: implementar disconnect()
    // TODO: implementar hilo lector de mensajes (alerts en tiempo real)
    // TODO: implementar setMessageListener(MessageListener l)
}
