#include <Arduino.h>

#include <AudioTools.h>
#include <BluetoothA2DPSink.h>

BluetoothA2DPSink a2dp_sink;
I2SStream i2s;


HardwareSerial serial_bt(2);

unsigned long last_rssi_check = 0;


void serial_send(String str);

//https://github.com/pschatzmann/arduino-audio-tools/blob/main/examples/examples-communication/a2dp/basic-a2dp-i2s/basic-a2dp-i2s.ino

// Write data to I2S
void read_data_stream(const uint8_t *data, uint32_t length) 
{
  i2s.write(data, length);
}

void bluetoothsink_avrc_metadata_callback(uint8_t data1, const uint8_t *data2)
{
  char bluetooth_media_title[255];
  char bluetooth_media_artist[255];

  Serial.printf("AVRC metadata rsp: attribute id 0x%x, %s\n", data1, data2);

  //serial_bt.printf("AVRC metadata rsp: attribute id 0x%x, %s\n", data1, data2);

  if (data1 == 0x1) // Title
  {        
      strncpy(bluetooth_media_title, (char *)data2, sizeof(bluetooth_media_title) - 1);

      serial_send("AT+TITLE=" + String(bluetooth_media_title));
  }
  else if (data1 == 0x2) // Artist
  {
      strncpy(bluetooth_media_artist, (char *)data2, sizeof(bluetooth_media_artist) - 1);
      
      serial_send("AT+ARTIST=" + String(bluetooth_media_artist));
      
  }
}

void connection_state_changed(esp_a2d_connection_state_t state, void *ptr)
{
  Serial.println(a2dp_sink.to_str(state));
  serial_send("AT+CONNSTATE=" + String(a2dp_sink.to_str(state)));
}

// for esp_a2d_audio_state_t see https://docs.espressif.com/projects/esp-idf/en/latest/esp32/api-reference/bluetooth/esp_a2dp.html#_CPPv421esp_a2d_audio_state_t
void audio_state_changed(esp_a2d_audio_state_t state, void *ptr)
{
  Serial.println(String((int)state));
  String str = "";
  switch(state)
  {
    case esp_a2d_audio_state_t::ESP_A2D_AUDIO_STATE_REMOTE_SUSPEND:
      str = "PAUSED";
      break;
    case esp_a2d_audio_state_t::ESP_A2D_AUDIO_STATE_STOPPED:
      str = "STOPPED";
      break;
    case esp_a2d_audio_state_t::ESP_A2D_AUDIO_STATE_STARTED:
      str = "PLAYING";
      break;


  }
  serial_send("AT+AUDIOSTATE=" + str);
}

void bt_rssi_changed(esp_bt_gap_cb_param_t::read_rssi_delta_param &rssi)
{
  static int8_t last_rssi;
  Serial.print("RSSI: ");
  Serial.print(rssi.rssi_delta);
  Serial.println(" dBm");

  if(last_rssi != rssi.rssi_delta)
    serial_send("AT+RSSI=" + String(rssi.rssi_delta));

  last_rssi = rssi.rssi_delta;
}

void setup() 
{
  Serial.begin(115200);
//  AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Error);

  Serial.println("BT slave");

  serial_bt.begin(115200, SERIAL_8N1, 16, 27);

  serial_send("AT+READY");
  

  // register callback
  a2dp_sink.set_stream_reader(read_data_stream, false);
  a2dp_sink.set_avrc_metadata_callback(bluetoothsink_avrc_metadata_callback);
  a2dp_sink.set_on_connection_state_changed(connection_state_changed);
  a2dp_sink.set_on_audio_state_changed(audio_state_changed);
  a2dp_sink.set_rssi_callback(bt_rssi_changed);
  
  a2dp_sink.set_rssi_active(true);  
  a2dp_sink.set_auto_reconnect(true);

  // setup I2S
  auto cfg = i2s.defaultConfig();
  
  cfg.rx_tx_mode = RxTxMode::TX_MODE;
  cfg.pin_data = 23; // SD OK
  cfg.pin_ws = 17; // WS OK
  cfg.pin_bck = 33;// SCK OK
  
  cfg.sample_rate = a2dp_sink.sample_rate();
  cfg.channels = 2;
  cfg.bits_per_sample = 16;
  cfg.buffer_count = 8;
  cfg.buffer_size = 256;
  i2s.begin(cfg);
}

void command_parse(String command)
{
  if(command == "AT+RESET")
  {
    Serial.println("I will reset!");
    delay(100);
    ESP.restart();
  }

  else if(command == "AT+PAUSE")
  {
    a2dp_sink.pause();
  }

  else if(command == "AT+PLAY")
  {
    a2dp_sink.play();    
  }

  else if(command == "AT+END")
  {
    a2dp_sink.end();
    delay(100);
    ESP.restart(); // Restart ESP, or auto-reconnect will not work
  }

  else if(command == "AT+START")
  {
    a2dp_sink.start("a2dp-i2s");
  }

}

void serial_send(String str)
{
  serial_bt.print(str + '\n');
}

void loop() 
{
   delay(100); 

  if(serial_bt.available() > 0)
  {
    String str = serial_bt.readStringUntil('\n');      
    
    Serial.println("Recv: "  + str);
    command_parse(str);
  }

  if (a2dp_sink.is_connected() && (millis() - last_rssi_check) > 3000) 
  {
    a2dp_sink.update_rssi();          
    last_rssi_check = millis();
  }
}