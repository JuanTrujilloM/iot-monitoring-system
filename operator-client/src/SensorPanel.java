import javax.swing.*;
import javax.swing.table.DefaultTableModel;

/**
 * SensorPanel.java - Panel que muestra sensores activos y sus mediciones
 *
 * Columnas de la tabla:
 *   ID | Tipo | Último valor | Unidad | Última actualización
 *
 * Se actualiza al recibir respuestas LIST SENSORS o mensajes DATA del servidor.
 */
public class SensorPanel extends JPanel {

    private JTable table;
    private DefaultTableModel tableModel;

    public SensorPanel() {
        // TODO: crear tabla con columnas
        // TODO: añadir JScrollPane
        // TODO: botón "Actualizar" que envía LIST SENSORS
    }

    public void updateSensor(String id, String type, String value, String unit, String timestamp) {
        // TODO: actualizar o insertar fila en la tabla (thread-safe con SwingUtilities.invokeLater)
    }

    public void clearSensors() {
        // TODO: limpiar todas las filas
    }
}
