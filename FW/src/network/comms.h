#ifndef COMMS_H
#define COMMS_H

#include <Arduino.h>

// Sends the CSV data file (located on SPIFFS at "/data.csv") over HTTP POST.
// serverURL: the full URL (including protocol and port) of the backend endpoint.
// Returns true if the POST request was successful.
bool sendCSVData(const char* serverURL);

#endif // COMMS_H
