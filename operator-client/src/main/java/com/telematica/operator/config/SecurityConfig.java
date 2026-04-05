package com.telematica.operator.config;

import com.telematica.operator.service.AuthService;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.Configuration;
import org.springframework.security.authentication.AuthenticationProvider;
import org.springframework.security.authentication.BadCredentialsException;
import org.springframework.security.authentication.UsernamePasswordAuthenticationToken;
import org.springframework.security.core.Authentication;
import org.springframework.security.core.authority.SimpleGrantedAuthority;
import org.springframework.security.config.annotation.web.builders.HttpSecurity;
import org.springframework.security.config.annotation.web.configuration.EnableWebSecurity;
import org.springframework.security.web.SecurityFilterChain;

import java.util.List;

@Configuration
@EnableWebSecurity
public class SecurityConfig {

    private final AuthService authService;

    public SecurityConfig(AuthService authService) {
        this.authService = authService;
    }

    @Bean
    public SecurityFilterChain filterChain(HttpSecurity http) throws Exception {
        http
            .authorizeHttpRequests(auth -> auth
                .requestMatchers("/login", "/css/**", "/js/**").permitAll()
                .anyRequest().authenticated()
            )
            .formLogin(form -> form
                .loginPage("/login")
                .defaultSuccessUrl("/dashboard", true)
                .failureUrl("/login?error")
                .permitAll()
            )
            .logout(logout -> logout
                .logoutSuccessUrl("/login?logout")
                .permitAll()
            )
            // CSRF disabled for compatibility with WebSocket STOMP
            .csrf(csrf -> csrf.disable())
            .authenticationProvider(authProvider());

        return http.build();
    }

    @Bean
    public AuthenticationProvider authProvider() {
        return new AuthenticationProvider() {
            @Override
            public Authentication authenticate(Authentication auth) {
                String username = auth.getName();
                String password = auth.getCredentials().toString();

                String role = authService.validate(username, password);
                if (role == null) throw new BadCredentialsException("Credenciales inválidas");

                return new UsernamePasswordAuthenticationToken(
                    username, null,
                    List.of(new SimpleGrantedAuthority("ROLE_" + role.toUpperCase()))
                );
            }

            @Override
            public boolean supports(Class<?> type) {
                return UsernamePasswordAuthenticationToken.class.isAssignableFrom(type);
            }
        };
    }
}
