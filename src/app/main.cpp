#include "bootstrap.h"

Bootstrap bootstrap;
Application &app = bootstrap.application();

void setup()
{
  Serial.begin(SERIAL_BAUD_RATE);
  delay(SERIAL_STARTUP_DELAY_MS);
  
  app.setup();
}

void loop()
{
  app.loop();
}
