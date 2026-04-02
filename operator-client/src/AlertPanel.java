import javax.swing.*;
import java.awt.*;
import java.time.LocalDateTime;
import java.time.format.DateTimeFormatter;

/**
 * AlertPanel.java - Panel que muestra alertas recibidas en tiempo real
 *
 * Las alertas llegan como mensajes ALERT desde el servidor:
 *   ALERT <sensor_id> <tipo_alerta> <valor>
 *
 * Se muestran en una lista con timestamp y se resaltan en rojo si son críticas.
 */
public class AlertPanel extends JPanel {

    private DefaultListModel<String> listModel;
    private JList<String> alertList;

    public AlertPanel() {
        // TODO: crear JList con listModel
        // TODO: añadir JScrollPane con título "Alertas"
        // TODO: botón "Limpiar"
    }

    public void addAlert(String sensorId, String alertType, String value) {
        // TODO: agregar entrada con timestamp al listModel (thread-safe)
    }
}
