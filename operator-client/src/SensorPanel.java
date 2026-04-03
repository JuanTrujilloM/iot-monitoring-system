package src;

import javax.swing.*;
import java.awt.*;
import java.util.List;

public class SensorPanel extends JPanel {
    private JTextArea sensorDisplay;

    public SensorPanel() {
        setLayout(new BorderLayout());
        sensorDisplay = new JTextArea();
        sensorDisplay.setEditable(false);
        sensorDisplay.setFont(new Font("Monospaced", Font.PLAIN, 14));
        add(new JScrollPane(sensorDisplay), BorderLayout.CENTER);
    }

    public void updateSensorDisplay(List<String> sensors) {
        if (sensors.isEmpty()) {
            sensorDisplay.setText("No hay sensores conectados actualmente.");
            return;
        }

        StringBuilder sb = new StringBuilder();
        sb.append("Sensores Activos en el Sistema:\n");
        sb.append("--------------------------------------------------\n");

        for (String s : sensors) {
            String[] info = s.split(",");
            if (info.length >= 4) {
                sb.append("ID: ").append(info[0])
                  .append(" \tTipo: ").append(info[1])
                  .append(" \tValor: ").append(info[2])
                  .append(" ").append(info[3]).append("\n");
            }
        }
        
        sensorDisplay.setText(sb.toString());
    }
}