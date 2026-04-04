package com.telematica.operator.controller;

import com.telematica.operator.model.SensorData;
import com.telematica.operator.service.ServerConnectionService;
import org.springframework.security.core.Authentication;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.ResponseBody;

import java.util.List;

@Controller
public class DashboardController {

    private final ServerConnectionService connectionService;

    public DashboardController(ServerConnectionService connectionService) {
        this.connectionService = connectionService;
    }

    @GetMapping("/")
    public String root() {
        return "redirect:/dashboard";
    }

    @GetMapping("/dashboard")
    public String dashboard(Authentication auth, Model model) {
        model.addAttribute("username", auth.getName());
        model.addAttribute("connected", connectionService.isConnected());
        return "dashboard";
    }

    /** REST endpoint — carga inicial al abrir la página. */
    @GetMapping("/api/sensors")
    @ResponseBody
    public List<SensorData> getSensors() {
        return connectionService.getSensors();
    }
}
