
struct PacketInfo {
  //String time = "Waiting";
  float rssi = 0;
  float snr = 0;
  float frequencyerror = 0;    // Hz 
  bool crc_error = false;
};

struct ModemInfo {
  char satellite[25]  = "Norbi";
  String  modem_mode  = "LoRa" ;     // 1-LoRa  2-FSK  3-GMSK
  float   frequency   = 436.703; // MHz  
  float   freqOffset  = 0;       // Hz 
  float   bw          = 250.0;   // kHz dual sideban
  uint8_t sf          = 10 ;
  uint8_t cr          = 5 ;
  uint8_t sw          = 18;
  int8_t  power       = 5 ;
  uint16_t preambleLength = 8;
  float    bitrate     = 9.6 ;
  float   freqDev     = 5.0;
  uint8_t    OOK      = false; // 0 disable  1 -> 0.3  2-> 0.5  3 -> 0.6  4-> 1.0
  bool    crc         = true;
  uint8_t fldro       = true;
  uint8_t gain        = 0;
  uint32_t  NORAD     = 46494;  
  uint8_t   fsw[8]    = {0,0,0,0,0,0,0,0};
  uint8_t   swSize     = 0;
  uint8_t   filter[8] = {0,0,0,0,0,0,0,0};
};
struct Status {
  //bool mqtt_connected = false;
  bool radio_ready = false;
  PacketInfo lastPacketInfo;
  ModemInfo modeminfo;
};
