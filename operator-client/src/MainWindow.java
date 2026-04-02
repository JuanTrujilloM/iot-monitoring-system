import javax.swing.*;
import java.awt.*;

/**
 * MainWindow.java - Ventana principal de la interfaz gráfica del operador
 *
 * Paneles:
 *   - SensorPanel  : lista de sensores activos con última medición
 *   - AlertPanel   : log de alertas recibidas en tiempo real
 *   - Barra de estado: conexión, usuario autenticado, hora
 *
 * Actualiza la UI cuando ServerConnection notifica nuevos mensajes.
 */
public class MainWindow extends JFrame {

    private SensorPanel sensorPanel;
    private AlertPanel alertPanel;
    private ServerConnection connection;

    public MainWindow(ServerConnection connection, String username, String role) {
        this.connection = connection;
        // TODO: configurar JFrame (título, tamaño, layout)
        // TODO: instanciar y añadir SensorPanel y AlertPanel
        // TODO: registrar MessageListener en connection
        // TODO: enviar LIST SENSORS al conectar para cargar estado inicial
    }

    // TODO: implementar onMessageReceived(String msg) — parsear y despachar a paneles
    // TODO: implementar refreshSensors()
}
