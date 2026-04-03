package src;

import javax.swing.*;
import java.awt.*;
import java.awt.event.ActionEvent;

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
                dispose();
                openMainWindow();
            } else {
                JOptionPane.showMessageDialog(this, "Credenciales inválidas", "Acceso Denegado", JOptionPane.ERROR_MESSAGE);
            }
        } catch (Exception ex) {
            JOptionPane.showMessageDialog(this, "Error de red", "Error", JOptionPane.ERROR_MESSAGE);
        }
    }

    private void openMainWindow() {
        try {
            MainWindow window = new MainWindow();

            String sensorResponse = connection.fetchSensors();
            window.getSensorPanel().updateSensorDisplay(connection.parseSensors(sensorResponse));

            String alertResponse = connection.fetchAlerts();
            window.getAlertPanel().updateAlertDisplay(connection.parseAlerts(alertResponse));

            window.setVisible(true);
        } catch (Exception ex) {
            ex.printStackTrace();
        }
    }
}