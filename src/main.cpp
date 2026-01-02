#include <Arduino.h>

#include <AudioTools.h>
#include <BluetoothA2DPSink.h>

BluetoothA2DPSink a2dp_sink;
I2SStream i2s;

//https://github.com/pschatzmann/arduino-audio-tools/blob/main/examples/examples-communication/a2dp/basic-a2dp-i2s/basic-a2dp-i2s.ino

// Write data to I2S
void read_data_stream(const uint8_t *data, uint32_t length) {
  i2s.write(data, length);
}

void bluetoothsink_avrc_metadata_callback(uint8_t data1, const uint8_t *data2)
{
  char bluetooth_media_title[255];
//a2dp_sink.set_auto_reconnect(true,  1000);
    Serial.printf("AVRC metadata rsp: attribute id 0x%x, %s\n", data1, data2);

    if (data1 == 0x1)
    {
        // Title
        strncpy(bluetooth_media_title, (char *)data2, sizeof(bluetooth_media_title) - 1);
        Serial.write(bluetooth_media_title);
    }
    else if (data1 == 0x2)
    {
        strncat(bluetooth_media_title, " - ", sizeof(bluetooth_media_title) - 1);
        strncat(bluetooth_media_title, (char *)data2, sizeof(bluetooth_media_title) - 1);
        
        Serial.write(bluetooth_media_title);
        
    }
}

void setup() {
  Serial.begin(115200);
//  AudioToolsLogger.begin(Serial, AudioToolsLogLevel::Error);

  Serial.println("BT slave");
  

  // register callback
  a2dp_sink.set_stream_reader(read_data_stream, false);
  a2dp_sink.set_avrc_metadata_callback(bluetoothsink_avrc_metadata_callback);

  // Start Bluetooth Audio Receiver
  a2dp_sink.set_auto_reconnect(true);
  a2dp_sink.start("a2dp-i2s");

  // setup output
  auto cfg = i2s.defaultConfig();
  
  //cfg.pin_mck = 23;
  cfg.rx_tx_mode = RxTxMode::TX_MODE;

  //cfg.i2s_format
  
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

void loop() 
{
   delay(100); 
}