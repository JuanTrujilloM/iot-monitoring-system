package src;

import javax.swing.*;

public class Main {
    public static void main(String[] args) {
        SwingUtilities.invokeLater(() -> {
            try {
                ServerConnection connection = new ServerConnection();
                connection.connect("127.0.0.1", 8080);

                LoginWindow loginWindow = new LoginWindow(connection);
                loginWindow.setVisible(true);

            } catch (Exception e) {
                JOptionPane.showMessageDialog(null, 
                    "No se pudo conectar al servidor central de monitoreo.", 
                    "Error Crítico de Red", 
                    JOptionPane.ERROR_MESSAGE);
            }
        });
    }
}