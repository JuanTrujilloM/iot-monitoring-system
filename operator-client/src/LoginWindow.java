package src;

import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;
import java.io.IOException;

public class LoginWindow extends JFrame {
    private JTextField userField;
    private JPasswordField passField;
    private ServerConnection connection;

    public LoginWindow(ServerConnection connection) {
        this.connection = connection;
        setTitle("Autenticación del Sistema");
        setSize(350, 200);
        setDefaultCloseOperation(JFrame.EXIT_ON_CLOSE);
        setLocationRelativeTo(null);

        JPanel panel = new JPanel(new GridLayout(3, 2, 10, 10));
        panel.setBackground(new Color(20, 20, 20));
        panel.setBorder(BorderFactory.createEmptyBorder(20, 20, 20, 20));

        JLabel userLabel = new JLabel("Usuario:");
        userLabel.setForeground(Color.WHITE);
        panel.add(userLabel);

        userField = new JTextField();
        userField.setBackground(new Color(40, 40, 40));
        userField.setForeground(Color.WHITE);
        userField.setCaretColor(Color.WHITE);
        userField.setBorder(BorderFactory.createLineBorder(new Color(180, 30, 30)));
        panel.add(userField);

        JLabel passLabel = new JLabel("Contraseña:");
        passLabel.setForeground(Color.WHITE);
        panel.add(passLabel);

        passField = new JPasswordField();
        passField.setBackground(new Color(40, 40, 40));
        passField.setForeground(Color.WHITE);
        passField.setCaretColor(Color.WHITE);
        passField.setBorder(BorderFactory.createLineBorder(new Color(180, 30, 30)));
        panel.add(passField);

        panel.add(new JLabel(""));

        JButton loginBtn = new JButton("INGRESAR");
        loginBtn.setBackground(new Color(180, 30, 30));
        loginBtn.setForeground(Color.WHITE);
        loginBtn.setFocusPainted(false);
        loginBtn.setFont(new Font("SansSerif", Font.BOLD, 12));
        loginBtn.addActionListener(this::attemptLogin);
        panel.add(loginBtn);

        add(panel);
    }

    private void attemptLogin(ActionEvent e) {
        String user = userField.getText();
        String pass = new String(passField.getPassword());

        try {
            if (connection.login(user, pass)) {
                String role = connection.getUserRole();
                
                // Enviar identificación como operador supervisor al servidor
                try {
                    if (connection.identifyAsOperator()) {
                        JOptionPane.showMessageDialog(this, 
                            "Conectado como operador supervisor: " + role.toUpperCase(), 
                            "Autenticación Exitosa", 
                            JOptionPane.INFORMATION_MESSAGE);
                        dispose();
                        openMainWindow();
                    }
                } catch (IOException identifyError) {
                    JOptionPane.showMessageDialog(this, 
                        "Error al identificarse como operador: " + identifyError.getMessage(), 
                        "Error de Identificación", 
                        JOptionPane.ERROR_MESSAGE);
                }
            }
        } catch (IOException ex) {
            // Capturar excepciones específicas de login
            String errorMsg = ex.getMessage();
            if (errorMsg != null && errorMsg.contains("Credenciales inválidas")) {
                JOptionPane.showMessageDialog(this, 
                    "Usuario o contraseña incorrectos", 
                    "Acceso Denegado", 
                    JOptionPane.ERROR_MESSAGE);
            } else if (errorMsg != null && errorMsg.contains("no disponible")) {
                JOptionPane.showMessageDialog(this, 
                    "Servicio de autenticación no disponible.\nIntente más tarde.", 
                    "Servicio Indisponible", 
                    JOptionPane.WARNING_MESSAGE);
            } else if (errorMsg != null && errorMsg.contains("No se pudo conectar")) {
                JOptionPane.showMessageDialog(this, 
                    "No se puede conectar al servidor.\nVerifique IP y puerto.", 
                    "Error de Conexión", 
                    JOptionPane.ERROR_MESSAGE);
            } else {
                JOptionPane.showMessageDialog(this, 
                    "Error: " + (errorMsg != null ? errorMsg : "Error desconocido"), 
                    "Error de Comunicación", 
                    JOptionPane.ERROR_MESSAGE);
            }
        }
    }

    private void openMainWindow() {
        try {
            MainWindow window = new MainWindow();

            // Callbacks despachados al hilo lector — actualizar UI en EDT
            connection.setOnSensorsUpdate(sensors ->
                SwingUtilities.invokeLater(() ->
                    window.getSensorPanel().updateSensorDisplay(sensors)));

            connection.setOnAlertReceived(alertLine ->
                SwingUtilities.invokeLater(() ->
                    window.getAlertPanel().addAlert(alertLine)));

            connection.setOnConnectionLost(() ->
                SwingUtilities.invokeLater(() ->
                    JOptionPane.showMessageDialog(window,
                        "Se perdió la conexión con el servidor.\n" +
                        "La supervisión en tiempo real se detuvo.",
                        "Conexión interrumpida",
                        JOptionPane.WARNING_MESSAGE)));

            // Iniciar hilo lector antes de pedir sensores
            connection.startListening();

            // Timer solo manda GET_SENSORS cada 2s; la respuesta la procesa el hilo lector
            Timer timer = new Timer(2000, e -> connection.requestSensors());
            timer.start();

            window.setVisible(true);
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }
}