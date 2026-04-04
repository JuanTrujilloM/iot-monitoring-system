package src;

import java.io.*;
import java.net.*;
import java.util.*;
import java.util.function.Consumer;

public class ServerConnection {
    private Socket socket;
    private PrintWriter out;
    private BufferedReader in;
    private String userRole;
    private volatile boolean running = false;

    private Consumer<List<String>> onSensorsUpdate;
    private Consumer<String>       onAlertReceived;
    private Runnable               onConnectionLost;

    public void connect(String host, int port) throws IOException {
        try {
            socket = new Socket(host, port);
            out = new PrintWriter(socket.getOutputStream(), true);
            in  = new BufferedReader(new InputStreamReader(socket.getInputStream()));
        } catch (ConnectException e) {
            throw new IOException("No se pudo conectar al servidor en " + host + ":" + port, e);
        } catch (SocketTimeoutException e) {
            throw new IOException("Timeout al conectar con el servidor", e);
        }
    }

    public void setOnSensorsUpdate(Consumer<List<String>> cb) { this.onSensorsUpdate = cb; }
    public void setOnAlertReceived(Consumer<String> cb)       { this.onAlertReceived  = cb; }
    public void setOnConnectionLost(Runnable cb)              { this.onConnectionLost = cb; }

    /**
     * Inicia el hilo lector que despacha todos los mensajes entrantes.
     * Llamar DESPUÉS de login/identify.
     */
    public void startListening() {
        running = true;
        Thread reader = new Thread(() -> {
            try {
                String line;
                while (running && (line = in.readLine()) != null) {
                    final String msg = line;
                    if (msg.startsWith("SENSORS") && onSensorsUpdate != null) {
                        onSensorsUpdate.accept(parseSensors(msg));
                    } else if (msg.startsWith("ALERTA") && onAlertReceived != null) {
                        onAlertReceived.accept(msg);
                    }
                    // OK, PONG, etc. se ignoran — solo interesan SENSORS y ALERTA
                }
            } catch (IOException e) {
                if (running && onConnectionLost != null) {
                    onConnectionLost.run();
                }
            }
        }, "socket-reader");
        reader.setDaemon(true);
        reader.start();
    }

    public void stopListening() {
        running = false;
    }

    /** Envía GET_SENSORS; la respuesta la procesa el hilo lector. */
    public void requestSensors() {
        out.print("GET_SENSORS\r\n");
        out.flush();
    }

    // ── Login e identificación (síncronos, antes de startListening) ────────

    public boolean login(String username, String password) throws IOException {
        out.print("LOGIN " + username + " " + password + "\r\n");
        out.flush();
        String response = in.readLine();

        if (response == null) throw new IOException("El servidor cerró la conexión sin responder");

        if (response.startsWith("ERROR")) {
            String[] parts = response.split(" ", 3);
            String code = parts.length >= 2 ? parts[1] : "?";
            if ("401".equals(code)) throw new IOException("Credenciales inválidas");
            if ("503".equals(code)) throw new IOException("Servicio de autenticación no disponible");
            throw new IOException("Error del servidor: " + code);
        }

        if (response.startsWith("OK")) {
            String[] parts = response.split(" ", 2);
            if (parts.length >= 2 && parts[1].trim().startsWith("ROLE_")) {
                userRole = parts[1].trim().substring(5);
                return true;
            }
        }
        return false;
    }

    public boolean identifyAsOperator() throws IOException {
        out.print("OPERATOR_IDENTIFY\r\n");
        out.flush();
        String response = in.readLine();
        if (response == null) throw new IOException("El servidor cerró la conexión durante identificación");
        return response.startsWith("OK");
    }

    public String getUserRole() { return userRole; }

    // ── Parsing ────────────────────────────────────────────────────────────

    public List<String> parseSensors(String response) {
        List<String> sensors = new ArrayList<>();
        if (response == null || !response.startsWith("SENSORS ")) return sensors;
        String data = response.substring(8);
        for (String part : data.split(";")) {
            if (!part.trim().isEmpty()) sensors.add(part.trim());
        }
        return sensors;
    }
}
