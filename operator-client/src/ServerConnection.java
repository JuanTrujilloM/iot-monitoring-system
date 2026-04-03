package src;

import java.io.*;
import java.net.*;
import java.util.*;

public class ServerConnection {
    private Socket socket;
    private PrintWriter out;
    private BufferedReader in;
    private String userRole;  // Almacenar el rol del operador autenticado

    public void connect(String host, int port) throws IOException {
        try {
            socket = new Socket(host, port);
            out = new PrintWriter(socket.getOutputStream(), true);
            in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
        } catch (ConnectException e) {
            throw new IOException("No se pudo conectar al servidor en " + host + ":" + port, e);
        } catch (SocketTimeoutException e) {
            throw new IOException("Timeout al conectar con el servidor", e);
        }
    }

    public boolean login(String username, String password) throws IOException {
        out.print("LOGIN " + username + " " + password + "\r\n");
        out.flush();
        String response = in.readLine();
        
        if (response == null) {
            throw new IOException("El servidor cerró la conexión sin responder");
        }
        
        // Verificar si es una respuesta de error
        if (response.startsWith("ERROR")) {
            String[] parts = response.split(" ", 3);
            if (parts.length >= 2) {
                String errorCode = parts[1];
                if ("401".equals(errorCode)) {
                    throw new IOException("Credenciales inválidas");
                } else if ("503".equals(errorCode)) {
                    throw new IOException("Servicio de autenticación no disponible");
                } else {
                    throw new IOException("Error del servidor: " + errorCode);
                }
            }
        }
        
        // Extraer el rol de la respuesta "OK ROLE_operator"
        if (response.startsWith("OK")) {
            String[] parts = response.split(" ", 2);
            if (parts.length >= 2) {
                String roleInfo = parts[1].trim();
                if (roleInfo.startsWith("ROLE_")) {
                    userRole = roleInfo.substring(5);  // Extraer solo "operator", "admin", etc.
                    return true;
                }
            }
        }
        
        return false;
    }
    
    /**
     * Obtiene el rol del operador autenticado
     * @return El rol (ej: "operator", "admin") o null si no autenticado
     */
    public String getUserRole() {
        return userRole;
    }
    
    /**
     * Verifica si el usuario tiene un rol específico
     * @param expectedRole El rol esperado
     * @return true si el rol coincide
     */
    public boolean hasRole(String expectedRole) {
        return userRole != null && userRole.equals(expectedRole);
    }

    /**
     * Envía comando de identificación del operador supervisor al servidor
     * @return true si el servidor confirmó la identificación
     * @throws IOException si hay error en la comunicación
     */
    public boolean identifyAsOperator() throws IOException {
        out.print("OPERATOR_IDENTIFY\r\n");
        out.flush();
        String response = in.readLine();
        
        if (response == null) {
            throw new IOException("El servidor cerró la conexión durante identificación");
        }
        
        if (response.startsWith("OK")) {
            return true;
        } else if (response.startsWith("ERROR")) {
            String[] parts = response.split(" ", 3);
            if (parts.length >= 2) {
                throw new IOException("Error en identificación: " + parts[1]);
            }
        }
        return false;
    }

    public String fetchSensors() throws IOException {
        out.print("GET_SENSORS\r\n");
        out.flush();
        return in.readLine();
    }

    public List<String> parseSensors(String response) {
        List<String> sensors = new ArrayList<>();
        
        if (response == null || !response.startsWith("SENSORS ") || response.contains("NONE")) {
            return sensors;
        }
        
        String data = response.substring(8);
        String[] parts = data.split(";");
        
        for (String part : parts) {
            if (!part.trim().isEmpty()) {
                sensors.add(part);
            }
        }
        
        return sensors;
    }

    public String fetchAlerts() throws IOException {
        out.print("GET_ALERTS\r\n");
        out.flush();
        return in.readLine();
    }

    public List<String> parseAlerts(String response) {
        List<String> alerts = new ArrayList<>();
        
        if (response == null || !response.startsWith("ALERTS ") || response.contains("NONE")) {
            return alerts;
        }
        
        String data = response.substring(7);
        String[] parts = data.split(";");
        
        for (String part : parts) {
            if (!part.trim().isEmpty()) {
                alerts.add(part);
            }
        }
        
        return alerts;
    }
}