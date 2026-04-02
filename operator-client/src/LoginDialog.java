import javax.swing.*;
import java.awt.*;
import java.awt.event.*;

/**
 * LoginDialog.java - Diálogo de autenticación del operador
 *
 * Envía al servidor: AUTH <usuario> <password>
 * Espera respuesta:  OK <rol>  |  ERROR 401 Unauthorized
 *
 * Si el login es exitoso, expone el rol para que Main pueda abrir MainWindow.
 */
public class LoginDialog extends JDialog {

    private JTextField hostField;
    private JTextField portField;
    private JTextField userField;
    private JPasswordField passField;
    private boolean authenticated = false;
    private String role = "";

    public LoginDialog() {
        // TODO: configurar JDialog (modal, tamaño, título)
        // TODO: añadir campos host, port, usuario, contraseña
        // TODO: botón "Conectar" → llamar a doLogin()
    }

    private void doLogin() {
        // TODO: crear ServerConnection, conectar, enviar AUTH, leer respuesta
        // TODO: si OK → authenticated = true, guardar role, cerrar dialog
        // TODO: si ERROR → mostrar JOptionPane con el error
    }

    public boolean isAuthenticated() { return authenticated; }
    public String getRole() { return role; }
    // TODO: exponer getConnection() para reutilizarla en MainWindow
}
