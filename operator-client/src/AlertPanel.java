package src;

import javax.swing.*;
import javax.swing.text.*;
import java.awt.*;
import java.util.List;

public class AlertPanel extends JPanel {
    private JTextPane alertDisplay;

    public AlertPanel() {
        setLayout(new BorderLayout());
        alertDisplay = new JTextPane();
        alertDisplay.setEditable(false);
        alertDisplay.setBackground(new Color(30, 30, 30)); // Fondo oscuro para resaltar colores
        add(new JScrollPane(alertDisplay), BorderLayout.CENTER);
    }

    public void updateAlertDisplay(List<String> alerts) {
        alertDisplay.setText(""); // Limpiar vista anterior
        
        if (alerts.isEmpty()) {
            appendColoredText("INFO: El sistema está estable. No hay alertas activas.\n", Color.GREEN);
            return;
        }

        appendColoredText("--- ALERTAS ACTIVAS DEL SISTEMA ---\n\n", Color.WHITE);

        for (String a : alerts) {
            // El formato de tu servidor C es: sensor_id,sensor_type,value,level_str,timestamp,message;
            String[] info = a.split(",");
            if (info.length >= 6) {
                String level = info[3];
                Color color = Color.WHITE;

                // Criterio de aceptación: Las alertas se destacan visualmente
                if (level.equals("CRITICAL")) {
                    color = new Color(255, 85, 85); // Rojo claro
                } else if (level.equals("WARNING")) {
                    color = new Color(255, 184, 108); // Naranja/Amarillo
                }

                String text = String.format("[%s] Sensor %s (%s) | Valor: %s | Detalle: %s\n", 
                                          level, info[0], info[1], info[2], info[5]);
                appendColoredText(text, color);
            }
        }
    }

    private void appendColoredText(String text, Color color) {
        StyleContext sc = StyleContext.getDefaultStyleContext();
        AttributeSet aset = sc.addAttribute(SimpleAttributeSet.EMPTY, StyleConstants.Foreground, color);
        aset = sc.addAttribute(aset, StyleConstants.FontFamily, "Monospaced");
        aset = sc.addAttribute(aset, StyleConstants.FontSize, 14);
        aset = sc.addAttribute(aset, StyleConstants.Bold, true);

        try {
            // Insertamos directamente en el documento para evitar el bloqueo del setEditable(false)
            alertDisplay.getDocument().insertString(alertDisplay.getDocument().getLength(), text, aset);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }
}