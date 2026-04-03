package src;

import javax.swing.*;

public class Main {
    public static void main(String[] args) {
        SwingUtilities.invokeLater(() -> {
            ServerConnection connection = null;
            try {
                connection = new ServerConnection();
                // Conectarse al servidor central en localhost:8080
                connection.connect("127.0.0.1", 8080);

                // Mostrar la ventana de login
                LoginWindow loginWindow = new LoginWindow(connection);
                loginWindow.setVisible(true);

            } catch (Exception e) {
                String errorMsg = e.getMessage();
                String fullError = "No se pudo conectar al servidor central de monitoreo.";
                
                if (errorMsg != null) {
                    if (errorMsg.contains("Connection refused")) {
                        fullError += "\n\nEl servidor no está activo. Verifique que el servidor esté corriendo en puerto 8080.";
                    } else if (errorMsg.contains("No se pudo conectar")) {
                        fullError += "\n\n" + errorMsg;
                    } else if (errorMsg.contains("Timeout")) {
                        fullError += "\n\nTimeout: El servidor tardó demasiado en responder.";
                    } else {
                        fullError += "\n\nError: " + errorMsg;
                    }
                }
                
                JOptionPane.showMessageDialog(null, 
                    fullError, 
                    "Error Crítico de Red", 
                    JOptionPane.ERROR_MESSAGE);
                System.exit(1);
            }
        });
    }
}