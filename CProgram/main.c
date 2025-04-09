#include <windows.h>
#include <unistd.h>
#include <stdio.h>
#include <tchar.h>
#include <stdbool.h>
#include <time.h>
#include <sys/time.h>
#include <string.h>

#include <src/smpt_ll_client.h>

#define PIPE_NAME L"\\\\.\\pipe\\UnityToCPipe"
#define BUFFER_SIZE 1024
#define MAX_FILENAME_LENGTH 100
#define PORT "COM3"

double getCurrentUnixTimestamp(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec + (tv.tv_usec / 1000000.0);
}

void getTimestampedFilename(char* filename, size_t max_length) {
    time_t rawtime;
    struct tm timeinfo;
    char buffer[80];

    time(&rawtime);
    localtime_s(&timeinfo, &rawtime);
    strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", &timeinfo);

    snprintf(filename, max_length, "received_values_%s.csv", buffer);
}

void logToCSV(FILE *logFile, double timestamp, uint16_t microSeconds, float milliAmpere, float baseValue, bool hasBaseValue) {
    if (hasBaseValue) {
        fprintf(logFile, "%.3f;%hu;%.2f;%.2f\n", timestamp, microSeconds, milliAmpere, baseValue);
    } else {
        fprintf(logFile, "%.3f;%hu;%.2f\n", timestamp, microSeconds, milliAmpere);
    }
    fflush(logFile);
}

int main()
{
    Smpt_device smpt_device;
    Smpt_ll_init smpt_ll_init;
    Smpt_ll_channel_config smpt_ll_channel_config;
    Smpt_ll_point smpt_ll_point;
    double last_time = getCurrentUnixTimestamp();
    double new_time;

    smpt_ll_channel_config.enable_stimulation = true;
    smpt_ll_channel_config.channel = Smpt_Channel_Red;
    smpt_ll_channel_config.connector = Smpt_Connector_Yellow;
    smpt_ll_channel_config.number_of_points = 1;

    smpt_open_serial_port(&smpt_device, PORT);
    smpt_send_ll_init(&smpt_device, &smpt_ll_init);

    HANDLE hPipe;
    char asciiBuffer[BUFFER_SIZE];

    char filename[MAX_FILENAME_LENGTH];
    getTimestampedFilename(filename, MAX_FILENAME_LENGTH);

    FILE *logFile = fopen(filename, "w");
    if (logFile == NULL) {
        wprintf(L"Failed to open log file: %hs\n", filename);
        return 1;
    }

    // Write CSV header
    fprintf(logFile, "UnixTimestamp;Microseconds;MilliAmpere;BaseValue\n");

    hPipe = CreateNamedPipeW(
        PIPE_NAME, PIPE_ACCESS_INBOUND, PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
        1, BUFFER_SIZE * sizeof(WCHAR), BUFFER_SIZE * sizeof(WCHAR), 0, NULL);

    if (hPipe == INVALID_HANDLE_VALUE) {
        wprintf(L"CreateNamedPipe failed, error %d\n", GetLastError());
        fclose(logFile);
        return 1;
    }

    wprintf(L"Waiting for client connection...\n");

    BOOL fConnected = ConnectNamedPipe(hPipe, NULL) ?
                          TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

    if (fConnected) {
        wprintf(L"Client connected, waiting for data...\n");
        fflush(stdout);

        while (1) {
            DWORD bytesRead;
            if (ReadFile(hPipe, asciiBuffer, BUFFER_SIZE - 1, &bytesRead, NULL)) {
                asciiBuffer[bytesRead] = '\0';
                uint16_t microSeconds;
                float milliAmpere, baseValue;
                int valuesRead = sscanf(asciiBuffer, "%hu,%f,%f", &microSeconds, &milliAmpere, &baseValue);

                if (valuesRead >= 2) {
                    bool hasBaseValue = (valuesRead == 3);
                    double timestamp = getCurrentUnixTimestamp();

                    if (hasBaseValue) {
                        wprintf(L"Received at %.3f: %hu µs, %.2f mA, and %.2f base value\n", timestamp, microSeconds, milliAmpere, baseValue);
                    } else {
                        wprintf(L"Received at %.3f: %hu µs and %.2f mA\n", timestamp, microSeconds, milliAmpere);
                    }
                    fflush(stdout);

                    new_time = getCurrentUnixTimestamp();
                    printf("Time difference: %.3f seconds\n", new_time - last_time);
                    last_time = new_time;

                    logToCSV(logFile, timestamp, microSeconds, milliAmpere, baseValue, hasBaseValue);

                    if (microSeconds > 0 && microSeconds <= 4095 && milliAmpere >= 1 && milliAmpere <= 70) {
                        smpt_ll_point.time = microSeconds;
                        smpt_ll_point.current = milliAmpere;
                        smpt_ll_channel_config.points[0] = smpt_ll_point;
                        smpt_send_ll_channel_config(&smpt_device, &smpt_ll_channel_config);
                    }
                }
            }
            else {
                break;
            }
        }
    }

    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);
    fclose(logFile);
    sleep(1);
    smpt_close_serial_port(&smpt_device);

    return 0;
}
