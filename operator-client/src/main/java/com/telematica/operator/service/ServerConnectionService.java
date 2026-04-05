package com.telematica.operator.service;

import jakarta.annotation.PostConstruct;
import jakarta.annotation.PreDestroy;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.messaging.simp.SimpMessagingTemplate;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Service;

import com.telematica.operator.model.SensorData;

import java.io.*;
import java.net.Socket;
import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

/**
 * Singleton que mantiene la conexión TCP al servidor C.
 *
 * <ul>
 *   <li>On startup: it connects, performs LOGIN + OPERATOR_IDENTIFY with the
 *       service credentials defined in application.properties.</li>
 *   <li>Asynchronous reader thread: Processes incoming lines from the C server.
 *       – SENSORS  → updates sensor cache
 *       – ALERT   → publishes to WebSocket topic /topic/alerts</li>
 *   <li>@Scheduled: send GET_SENSORS every 2s to keep the cache fresh.</li>
 * </ul>
 */
@Service
public class ServerConnectionService {

    private static final Logger log = LoggerFactory.getLogger(ServerConnectionService.class);

    @Value("${iot.server.host}")
    private String serverHost;

    @Value("${iot.server.port}")
    private int serverPort;

    @Value("${iot.server.username}")
    private String serviceUsername;

    @Value("${iot.server.password}")
    private String servicePassword;

    private final SimpMessagingTemplate messagingTemplate;

    private Socket socket;
    private PrintWriter out;
    private BufferedReader in;
    private volatile boolean running = false;
    private volatile List<SensorData> cachedSensors = Collections.emptyList();

    public ServerConnectionService(SimpMessagingTemplate messagingTemplate) {
        this.messagingTemplate = messagingTemplate;
    }

    @PostConstruct
    public void connect() {
        try {
            socket = new Socket(serverHost, serverPort);
            out = new PrintWriter(socket.getOutputStream(), true);
            in  = new BufferedReader(new InputStreamReader(socket.getInputStream()));

            authenticate();
            startReaderThread();
            log.info("Conectado al servidor C en {}:{}", serverHost, serverPort);
        } catch (IOException e) {
            log.error("No se pudo conectar al servidor C: {}", e.getMessage());
        }
    }

    private void authenticate() throws IOException {
        out.print("LOGIN " + serviceUsername + " " + servicePassword + "\r\n");
        out.flush();
        String resp = in.readLine();
        if (resp == null || !resp.startsWith("OK")) {
            throw new IOException("LOGIN fallido: " + resp);
        }

        out.print("OPERATOR_IDENTIFY\r\n");
        out.flush();
        resp = in.readLine();
        if (resp == null || !resp.startsWith("OK")) {
            throw new IOException("OPERATOR_IDENTIFY fallido: " + resp);
        }
        log.info("Autenticado como operador supervisor");
    }

    private void startReaderThread() {
        running = true;
        Thread reader = new Thread(() -> {
            try {
                String line;
                while (running && (line = in.readLine()) != null) {
                    if (line.startsWith("SENSORS")) {
                        cachedSensors = parseSensors(line);
                        messagingTemplate.convertAndSend("/topic/sensors", cachedSensors);
                    } else if (line.startsWith("ALERTA")) {
                        messagingTemplate.convertAndSend("/topic/alerts", line);
                        log.debug("Alerta enviada a browsers: {}", line);
                    }
                }
            } catch (IOException e) {
                if (running) {
                    log.warn("Conexión con servidor C interrumpida: {}", e.getMessage());
                    messagingTemplate.convertAndSend("/topic/connection", "LOST");
                }
            }
        }, "c-server-reader");
        reader.setDaemon(true);
        reader.start();
    }

    /** Request a list of sensors; the response is processed by the reading thread. */
    @Scheduled(fixedDelay = 2000)
    public void pollSensors() {
        if (out != null && running) {
            out.print("GET_SENSORS\r\n");
            out.flush();
        }
    }

    public List<SensorData> getSensors() {
        return cachedSensors;
    }

    public boolean isConnected() {
        return running && socket != null && socket.isConnected() && !socket.isClosed();
    }

    private List<SensorData> parseSensors(String response) {
        List<SensorData> sensors = new ArrayList<>();
        if (!response.startsWith("SENSORS ")) return sensors;
        String data = response.substring(8);
        for (String part : data.split(";")) {
            String trimmed = part.trim();
            if (trimmed.isEmpty()) continue;
            String[] p = trimmed.split(",");
            sensors.add(new SensorData(
                p.length > 0 ? p[0].trim() : "?",
                p.length > 1 ? p[1].trim() : "?",
                p.length > 2 ? p[2].trim() : "?",
                p.length > 3 ? p[3].trim() : ""
            ));
        }
        return sensors;
    }

    @PreDestroy
    public void disconnect() {
        running = false;
        try {
            if (socket != null) socket.close();
        } catch (IOException ignored) {}
    }
}
