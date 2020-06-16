#include "Thorium.h"
#include <Preferences.h>

/* create an instance of Preferences library */
Preferences preferences;

// Timers
SimpleTimer thoriumClientCheckinTimer(30000);

// Create Thorium Data
struct thoriumClientInfo thoriumInfo;

const int WORD_LIST_SIZE = 138;
const char* words[] = {"alpha", "apastron", "aperture", "aphelion", "apogee", "asterism", "asteroid", "astronaut", "astronomer", "astronomy", "azimuth", "bolometer", "celestial", "centauri", "cislunar", "cluster", "comet", "conjunction", "constellation", "corona", "cosmology", "cosmonaut", "cosmos", "crater", "day", "declination", "deneb", "density", "docking", "dust", "earth", "earthbound", "eccentricity", "eclipse", "ecliptict", "equinox", "exoplanet", "extragalactic", "flare", "flyby", "galaxy", "gegenschein", "geostationary", "geosynchronous", "gravitation", "gravity", "heliocentric", "helium", "hydrogen", "hypernova", "inclination", "inertia", "interstellar", "ionosphere", "jupiter", "kiloparsec", "lens", "lunar", "magnitude", "mare", "mars", "mass", "mercury", "meteor", "meteorite", "meteoroid", "mir", "moon", "muttnik", "nadir", "nasa", "nebula", "neptune", "nova", "observatory", "occultation", "opposition", "orbit", "parallax", "parsec", "penumbra", "perigee", "perihelion", "perturbation", "phase", "planet", "planetary nebula", "planetoid", "pluto", "precession", "probe", "pulsar", "quasar", "radiant", "radiation", "revolve", "rings", "rocket", "satellite", "Saturn", "scintillation", "sidereal", "singularity", "sky", "solar", "solstice", "space", "spectroscope", "spectrum", "sputnik", "star", "starlight", "sun", "sunspot", "supernova", "synodic", "syzygy", "telemetry", "telescope", "terminator", "terrestrial", "totality", "transit", "translunar", "transneptunian", "twinkling", "umbra", "universe", "uranus", "vacuum", "venus", "waning", "wavelength", "waxing", "weightless", "wormhole", "zenith", "zodiac"};
const char* identifier = "hwpanel";

void thorium_setup() {
  generateClientId();
  graphql_registerClient();
  get_client_info();
}

void thorium_loop() {
  if (thoriumClientCheckinTimer.isReady()) { 
    get_client_info();
    thoriumClientCheckinTimer.reset();
  }
}

void get_client_info() {
  String json = graphql_request("query clients($client: ID!){ clients(clientId: $client){id, simulator { id, name }, flight {id, name } } }");
  DynamicJsonDocument doc(1024);
  StaticJsonDocument<200> filter;
  filter["data"]["clients"][0] = true;
  deserializeJson(doc, json, DeserializationOption::Filter(filter));
  JsonObject obj = doc.as<JsonObject>();
  
  Serial.print("ID: ");
  String id = obj["data"]["clients"][0]["id"];
  Serial.println(id);
  Serial.print("Flight: ");
  String flight = obj["data"]["clients"][0]["flight"]["id"];
  thoriumInfo.flightId = flight;
  Serial.println(thoriumInfo.flightId);
  Serial.print("Simulator: ");
  String simulator = obj["data"]["clients"][0]["simulator"]["id"];
  thoriumInfo.simulatorId = simulator;
  Serial.println(thoriumInfo.simulatorId);
}

String graphql_request(String postData) {
  struct thoriumResponse queryResponse = graphql_raw_request("{\"query\":\"" + postData +"\",\"variables\":{\"client\":\"" + thoriumInfo.clientId.string + "\",\"simulator\":\"" + thoriumInfo.simulatorId + "\"}}");
//  Serial.print("Status Code: ");
//  Serial.println(queryResponse.statusCode);
//  Serial.print("Response: ");
//  Serial.println(queryResponse.response);

  // TODO: write error handling. stuff like 400 status codes, etc. 
  
  return queryResponse.response;
}

String graphql_request(String postData, String variables) {
  struct thoriumResponse queryResponse = graphql_raw_request("{\"query\":\"" + postData +"\",\"variables\":{" + variables +",\"client\":\"" + thoriumInfo.clientId.string + "\",\"simulator\":\"" + thoriumInfo.simulatorId + "\"}}");
//  Serial.print("Status Code: ");
//  Serial.println(queryResponse.statusCode);
//  Serial.print("Response: ");
//  Serial.println(queryResponse.response);

  // TODO: write error handling. stuff like 400 status codes, etc. 
  
  return queryResponse.response;
}

struct thoriumResponse graphql_raw_request(String postData) {
  Serial.print("POST request: ");
  Serial.println(postData);
  String contentType = "application/json";

  httpclient.post("/graphql", contentType, postData);
  // read the status code and body of the response
  struct thoriumResponse queryResponse = { httpclient.responseStatusCode(), httpclient.responseBody() };

  return queryResponse;
}

void graphql_registerClient() {
  graphql_request("mutation RegisterClient($client: ID!) {\\n  clientConnect(client: $client, mobile:true, cards:[])\\n}");
}

void generateClientId() {
  preferences.begin("panelconfig", false);
  
  /* get value of key "reset_times", if key not exist return default value 0 in second argument
  Note: Key name is limited to 15 chars too */
  unsigned int id_word_one = preferences.getUInt("id_word_one", 0);
  unsigned int id_word_two = preferences.getUInt("id_word_two", 0);

  if(id_word_one + id_word_two == 0){
    // if analog input pin 0 is unconnected, random analog
    // noise will cause the call to randomSeed() to generate
    // different seed numbers each time the sketch runs.
    // randomSeed() will then shuffle the random function.
    randomSeed(analogRead(0));
  
    // print a random number from 0 to #WORD_LIST_SIZE
    // add 1 so that 0 can be marked as 'not set'
    id_word_one = random(WORD_LIST_SIZE) + 1;
    id_word_two = random(WORD_LIST_SIZE) + 1;
    Serial.println("Client ID not found. Generating a New Client ID. (Is this the first boot?)");

    /* Store reset_times to the Preferences */
    preferences.putUInt("id_word_one", id_word_one);
    preferences.putUInt("id_word_two", id_word_two);
  }
  /* Close the Preferences */
  preferences.end();
  // subtract 1 to remove 0 as your 'not set' number
  thoriumInfo.clientId  = { id_word_one - 1, id_word_two - 1, String(identifier) + "-" + words[id_word_one -1 ] + "-" + words[id_word_two - 1]};
  Serial.print("Client ID: ");
  Serial.print(thoriumInfo.clientId.string);
}
