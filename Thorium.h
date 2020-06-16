#ifndef _THORIUM_H 
#define _THORIUM_H

#include "communication.h"
#include <Preferences.h>
#include <ArduinoJson.h>
#include <SimpleTimer.h>

// define the max num of bytes for variable
// simulatorID is 36
#define HTTP_REQ_PARAM_VALUE_LENGTH 36

struct thoriumId {
   unsigned int word_one;
   unsigned int word_two;
   String string;
};
struct thoriumResponse {
  int statusCode;
  String response;
};

struct thoriumClientInfo {
  String flightId;
  String simulatorId;
  thoriumId clientId;
};

/* create an instance of Preferences library */
extern Preferences preferences;

// Timers
extern SimpleTimer thoriumClientCheckinTimer;

// Create Thorium Data
extern struct thoriumClientInfo thoriumInfo;

void thorium_setup();
void thorium_loop();
void get_client_info();
String graphql_request(String postData);
String graphql_request(String postData, String variables);
struct thoriumResponse graphql_raw_request(String postData);
void graphql_registerClient();
void generateClientId();

#endif // _THORIUM_H
