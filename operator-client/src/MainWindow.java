package src;

import javax.swing.*;
import java.awt.*;

public class MainWindow extends JFrame {
    private SensorPanel sensorPanel;
    private AlertPanel alertPanel;

    public MainWindow() {
        setTitle("Centro de Monitoreo IoT");
        setSize(700, 450);
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setLocationRelativeTo(null); // Centrar en la pantalla

        // Criterio de aceptación: La información se presenta de forma ordenada.
        JTabbedPane tabbedPane = new JTabbedPane();
        
        sensorPanel = new SensorPanel();
        alertPanel = new AlertPanel();

        tabbedPane.addTab("Sensores (Estado General)", sensorPanel);
        tabbedPane.addTab("Alertas y Notificaciones", alertPanel);

        add(tabbedPane, BorderLayout.CENTER);
    }

    public SensorPanel getSensorPanel() { return sensorPanel; }
    public AlertPanel getAlertPanel() { return alertPanel; }
}