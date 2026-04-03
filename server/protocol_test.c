/**
 * Protocol Test Program
 * 
 * This program demonstrates how the IoT protocol works by:
 * 1. Creating sample sensor messages
 * 2. Parsing and validating them
 * 3. Generating responses
 * 
 * Compile with: gcc -o protocol_test protocol_test.c src/protocol.c -Iinclude
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "protocol.h"

void print_message(const protocol_message_t *msg) {
	printf("  Sensor ID:   %d\n", msg->sensor_id);
	printf("  Type:        %s\n", protocol_get_sensor_type_name(msg->sensor_type));
	printf("  Value:       %.2f\n", msg->value);
	printf("  Timestamp:   %ld\n", msg->timestamp);
}

void test_case_1(void) {
	char buffer[256];
	protocol_message_t msg;
	protocol_response_t response;
	char response_buffer[256];
	
	printf("\n=== TEST CASE 1: Valid Temperature Sensor ===\n");
	
	/* Build a sensor message */
	protocol_message_t test_msg = {
		.type = MSG_TYPE_SENSOR_DATA,
		.sensor_id = 5,
		.sensor_type = SENSOR_TYPE_TEMPERATURE,
		.value = 22.3,
		.timestamp = time(NULL)
	};
	
	printf("Sensor message to send:\n");
	print_message(&test_msg);
	
	/* Build the message string */
	int msg_len = protocol_build_sensor_message(&test_msg, buffer, sizeof(buffer));
	printf("\nSerialized message: %s\n", buffer);
	
	/* Parse it back */
	if (protocol_parse_message(buffer, msg_len, &msg) == 0) {
		printf("\n✓ Message parsed successfully\n");
		printf("Parsed message:\n");
		print_message(&msg);
	} else {
		printf("\n✗ Failed to parse message\n");
		return;
	}
	
	/* Build response */
	response.status = STATUS_OK;
	response.sensor_id = msg.sensor_id;
	response.timestamp = time(NULL);
	strcpy(response.status_message, "OK");
	
	int resp_len = protocol_build_response(&response, response_buffer, sizeof(response_buffer));
	printf("\nServer response: %s\n", response_buffer);
}

void test_case_2(void) {
	char buffer[256];
	protocol_message_t msg;
	protocol_response_t response;
	char response_buffer[256];
	
	printf("\n=== TEST CASE 2: Out of Range Temperature ===\n");
	
	/* Try to send temperature that's too high */
	protocol_message_t test_msg = {
		.type = MSG_TYPE_SENSOR_DATA,
		.sensor_id = 7,
		.sensor_type = SENSOR_TYPE_TEMPERATURE,
		.value = 200.0,  /* Way above max of 150 */
		.timestamp = time(NULL)
	};
	
	printf("Sensor message (invalid value 200°C):\n");
	print_message(&test_msg);
	
	int msg_len = protocol_build_sensor_message(&test_msg, buffer, sizeof(buffer));
	printf("\nSerialized message: %s\n", buffer);
	
	/* Try to parse - should fail */
	if (protocol_parse_message(buffer, msg_len, &msg) != 0) {
		printf("✓ Message rejected (value out of range)\n");
		
		/* Build error response */
		response.status = STATUS_OUT_OF_RANGE;
		response.sensor_id = 7;
		response.timestamp = time(NULL);
		strcpy(response.status_message, protocol_get_status_message(STATUS_OUT_OF_RANGE));
		
		int resp_len = protocol_build_response(&response, response_buffer, sizeof(response_buffer));
		printf("\nServer response: %s\n", response_buffer);
	} else {
		printf("✗ Message should have been rejected!\n");
	}
}

void test_case_3(void) {
	char buffer[256];
	protocol_message_t msg;
	protocol_response_t response;
	char response_buffer[256];
	
	printf("\n=== TEST CASE 3: Energy Sensor (Valid) ===\n");
	
	protocol_message_t test_msg = {
		.type = MSG_TYPE_SENSOR_DATA,
		.sensor_id = 12,
		.sensor_type = SENSOR_TYPE_ENERGY,
		.value = 456.78,  /* kWh */
		.timestamp = time(NULL)
	};
	
	printf("Sensor message (Energy sensor):\n");
	print_message(&test_msg);
	
	int msg_len = protocol_build_sensor_message(&test_msg, buffer, sizeof(buffer));
	printf("\nSerialized message: %s\n", buffer);
	
	if (protocol_parse_message(buffer, msg_len, &msg) == 0) {
		printf("\n✓ Message parsed successfully\n");
		
		response.status = STATUS_OK;
		response.sensor_id = msg.sensor_id;
		response.timestamp = time(NULL);
		strcpy(response.status_message, "OK");
		
		int resp_len = protocol_build_response(&response, response_buffer, sizeof(response_buffer));
		printf("Server response: %s\n", response_buffer);
	}
}

void test_case_4(void) {
	char buffer[256];
	protocol_message_t msg;
	
	printf("\n=== TEST CASE 4: Malformed Message ===\n");
	
	printf("Sending malformed message (missing fields):\n");
	strcpy(buffer, "1:10:1");  /* Missing VALUE and TIMESTAMP */
	printf("Raw message: %s\n", buffer);
	
	if (protocol_parse_message(buffer, strlen(buffer), &msg) != 0) {
		printf("✓ Malformed message rejected\n");
	} else {
		printf("✗ Malformed message should have been rejected!\n");
	}
}

void test_case_5(void) {
	char buffer[256];
	protocol_message_t msg;
	
	printf("\n=== TEST CASE 5: Vibration Sensor (All Valid Ranges) ===\n");
	
	double test_values[] = {0.0, 25.5, 50.0, 75.5, 100.0};
	int test_values_count = sizeof(test_values) / sizeof(test_values[0]);
	
	for (int i = 0; i < test_values_count; i++) {
		protocol_message_t test_msg = {
			.type = MSG_TYPE_SENSOR_DATA,
			.sensor_id = 20 + i,
			.sensor_type = SENSOR_TYPE_VIBRATION,
			.value = test_values[i],
			.timestamp = time(NULL)
		};
		
		int msg_len = protocol_build_sensor_message(&test_msg, buffer, sizeof(buffer));
		
		if (protocol_parse_message(buffer, msg_len, &msg) == 0) {
			printf("✓ Vibration %.1f - Valid\n", test_values[i]);
		} else {
			printf("✗ Vibration %.1f - Invalid\n", test_values[i]);
		}
	}
}

int main(void) {
	printf("╔════════════════════════════════════════════════════════╗\n");
	printf("║     IoT Monitoring System - Protocol Test Suite       ║\n");
	printf("╚════════════════════════════════════════════════════════╝\n");
	
	test_case_1();
	test_case_2();
	test_case_3();
	test_case_4();
	test_case_5();
	
	printf("\n╔════════════════════════════════════════════════════════╗\n");
	printf("║              All tests completed                       ║\n");
	printf("╚════════════════════════════════════════════════════════╝\n");
	
	return 0;
}
