package com.telematica.operator.service;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Value;
import org.springframework.stereotype.Service;

import java.io.*;
import java.net.Socket;

/**
 * Valida credenciales contra el auth-service Python (puerto 9000).
 * Protocolo: enviar "usuario:password\n", recibir "ROLE:rol" o "ERROR:...".
 */
@Service
public class AuthService {

    private static final Logger log = LoggerFactory.getLogger(AuthService.class);

    @Value("${iot.auth.host}")
    private String authHost;

    @Value("${iot.auth.port}")
    private int authPort;

    /**
     * @return rol del usuario ("admin", "operador") o null si las credenciales son inválidas.
     */
    public String validate(String username, String password) {
        try (Socket socket = new Socket(authHost, authPort);
             PrintWriter out = new PrintWriter(socket.getOutputStream(), true);
             BufferedReader in = new BufferedReader(new InputStreamReader(socket.getInputStream()))) {

            out.println(username + ":" + password);
            String response = in.readLine();

            if (response != null && response.startsWith("ROLE:")) {
                return response.substring(5).trim();
            }
            log.warn("Auth failed for user '{}': {}", username, response);
        } catch (IOException e) {
            log.error("Auth service unreachable at {}:{} — {}", authHost, authPort, e.getMessage());
        }
        return null;
    }
}
