// #include <windows.h>
// #include <unistd.h>
// #include <stdio.h>
// #include <tchar.h>

// #include <src/smpt_ll_client.h>
// #include <sys/time.h>

// #define PIPE_NAME L"\\\\.\\pipe\\UnityToCPipe"
// #define BUFFER_SIZE 1024

// long long timeInMilliseconds(void) {
//     struct timeval tv;

//     gettimeofday(&tv,NULL);
//     return (((long long)tv.tv_sec)*1000)+(tv.tv_usec/1000);
// }

// int main()
// {
//     Smpt_device smpt_device;
//     Smpt_ll_init smpt_ll_init;
//     Smpt_ll_channel_config smpt_ll_channel_config;
//     Smpt_ll_point smpt_ll_point;
//     Smpt_ll_channel_config_ack smpt_ll_channel_config_ack;
//     long long last_time = timeInMilliseconds();
//     long long new_time = timeInMilliseconds();
//     long long time = 0;

//     // smpt_ll_point.time = 4095; //ms
//     // smpt_ll_point.current = 30; //mA;

//     smpt_ll_channel_config.enable_stimulation = true;
//     smpt_ll_channel_config.channel = Smpt_Channel_Red; //blue, black, white, undefined
//     smpt_ll_channel_config.connector = Smpt_Connector_Yellow; //green, undefined
//     smpt_ll_channel_config.number_of_points = 1;

//     smpt_open_serial_port(&smpt_device, "COM3");
//     smpt_send_ll_init(&smpt_device, &smpt_ll_init);




//     HANDLE hPipe;
//     WCHAR buffer[BUFFER_SIZE];
//     DWORD dwRead;

//     // Create the named pipe
//     hPipe = CreateNamedPipeW(
//         PIPE_NAME,                   // pipe name
//         PIPE_ACCESS_INBOUND,         // read access
//         PIPE_TYPE_BYTE |             // byte type pipe
//             PIPE_READMODE_BYTE |         // byte-read mode
//             PIPE_WAIT,                   // blocking mode
//         1,                           // number of instances
//         BUFFER_SIZE * sizeof(WCHAR), // output buffer size
//         BUFFER_SIZE * sizeof(WCHAR), // input buffer size
//         0,                           // client time-out
//         NULL);                       // default security attribute

//     if (hPipe == INVALID_HANDLE_VALUE) {
//         wprintf(L"CreateNamedPipe failed, error %d\n", GetLastError());
//         return 1;
//     }

//     wprintf(L"Waiting for client connection...\n");

//     // Wait for the client to connect
//     BOOL fConnected = ConnectNamedPipe(hPipe, NULL) ?
//                           TRUE : (GetLastError() == ERROR_PIPE_CONNECTED);

//     if (fConnected) {
//         wprintf(L"Client connected, waiting for data...\n");
//         fflush(stdout);


//         while (1) {
//             DWORD bytesRead;
//             char asciiBuffer[BUFFER_SIZE];
//             if (ReadFile(hPipe, asciiBuffer, BUFFER_SIZE - 1, &bytesRead, NULL)) {

//                 asciiBuffer[bytesRead] = '\0';  // Null-terminate the string
//                 uint16_t microSeconds;
//                 float milliAmpere;
//                 if (sscanf_s(asciiBuffer, "%hu,%f", &microSeconds, &milliAmpere) == 2) {
//                     wprintf(L"Received: %hu µs and %.2f mA\n", microSeconds, milliAmpere);
//                     fflush(stdout);
//                     new_time = timeInMilliseconds();
//                     printf("%llu\n", new_time - last_time);
//                     last_time = new_time;
//                     if (microSeconds > 0 && microSeconds <= 4095 && milliAmpere >= 1 && milliAmpere <= 70) {
//                         smpt_ll_point.time = microSeconds;
//                         smpt_ll_point.current = milliAmpere;
//                         smpt_ll_channel_config.points[0] = smpt_ll_point;
//                         smpt_send_ll_channel_config(&smpt_device, &smpt_ll_channel_config);
//                     }
//                 }
//             }
//             else {
//                 break;
//             }
//         }
//     }

//     // Disconnect and close the pipe
//     DisconnectNamedPipe(hPipe);
//     CloseHandle(hPipe);
//     sleep(1);
//     smpt_close_serial_port(&smpt_device);

//     return 0;




// }
