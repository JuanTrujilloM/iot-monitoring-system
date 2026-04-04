package src;

import javax.swing.*;
import javax.swing.text.*;
import java.awt.*;
import java.text.SimpleDateFormat;
import java.util.Date;

public class AlertPanel extends JPanel {
    private JTextPane alertDisplay;
    private int alertCount = 0;
    private static final SimpleDateFormat DATE_FORMAT = new SimpleDateFormat("dd/MM/yyyy HH:mm:ss");

    public AlertPanel() {
        setLayout(new BorderLayout());
        alertDisplay = new JTextPane();
        alertDisplay.setEditable(false);
        alertDisplay.setBackground(new Color(30, 30, 30));
        add(new JScrollPane(alertDisplay), BorderLayout.CENTER);

        appendColoredText("Sistema estable. Esperando alertas...\n", Color.GREEN);
    }

    /**
     * Agrega una alerta recibida por broadcast.
     * Formato del servidor: "ALERTA sensor_id,sensor_type,value,unit"
     */
    public void addAlert(String rawLine) {
        // quitar prefijo "ALERTA "
        String data = rawLine.startsWith("ALERTA ") ? rawLine.substring(7) : rawLine;
        String[] info = data.split(",");

        String timestamp = DATE_FORMAT.format(new Date());
        String sensorId  = info.length > 0 ? info[0].trim() : "?";
        String type      = info.length > 1 ? info[1].trim() : "?";
        String value     = info.length > 2 ? info[2].trim() : "?";
        String unit      = info.length > 3 ? info[3].trim() : "";

        // Determinar nivel comparando value contra umbrales conocidos por tipo
        Color color  = new Color(255, 184, 108); // WARNING por defecto
        String level = "WARNING";
        try {
            double v = Double.parseDouble(value);
            if ((type.equals("temperature") && v > 50) ||
                (type.equals("vibration")   && v > 10) ||
                (type.equals("energy")      && v > 800)) {
                color = new Color(255, 85, 85);
                level = "CRITICAL";
            }
        } catch (NumberFormatException ignored) {}

        alertCount++;
        String text = String.format("[%s] [%s] Sensor %s (%s) | Valor: %s %s\n",
                                    timestamp, level, sensorId, type, value, unit);
        appendColoredText(text, color);

        // Hacer scroll al final para ver la alerta nueva
        alertDisplay.setCaretPosition(alertDisplay.getDocument().getLength());
    }

    private void appendColoredText(String text, Color color) {
        StyleContext sc   = StyleContext.getDefaultStyleContext();
        AttributeSet aset = sc.addAttribute(SimpleAttributeSet.EMPTY, StyleConstants.Foreground, color);
        aset = sc.addAttribute(aset, StyleConstants.FontFamily, "Monospaced");
        aset = sc.addAttribute(aset, StyleConstants.FontSize, 14);
        aset = sc.addAttribute(aset, StyleConstants.Bold, true);
        try {
            alertDisplay.getDocument().insertString(alertDisplay.getDocument().getLength(), text, aset);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}
