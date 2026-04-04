#ifndef AUTH_CLIENT_H
#define AUTH_CLIENT_H

int auth_client_verify(const char* username, const char* password, char* role_buffer, int buffer_size);

#endif