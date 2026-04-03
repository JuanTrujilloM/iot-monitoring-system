package src;

import java.io.*;
import java.net.*;
import java.util.*;

public class ServerConnection {
    private Socket socket;
    private PrintWriter out;
    private BufferedReader in;

    public void connect(String host, int port) throws IOException {
        socket = new Socket(host, port);
        out = new PrintWriter(socket.getOutputStream(), true);
        in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
    }

    public boolean login(String username, String password) throws IOException {
        out.print("LOGIN " + username + " " + password + "\r\n");
        out.flush();
        String response = in.readLine();
        return response != null && response.startsWith("OK");
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