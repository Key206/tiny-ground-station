struct PacketInfo {
  String packet = "Empty";
  double lat = 0, lon = 0;
  uint8_t id = -1;
  float rssi = 0;
  float snr = 0;
  bool crc_error = false;
};
struct ModemInfo {
  String satellite  = "GaoFen";
  String  modem_mode  = "LoRa" ;    
  float   frequency   = 400.45; // MHz  // Norbi: 436.703
  float   freqOffset  = 0;       // Hz 
  float   bw          = 500;   // kHz dual sideban // Norbi: 250.0
  uint8_t sf          = 9 ;            // Norbi: 10
  uint8_t cr          = 5 ;            // Norbi: 5
  uint8_t sw          = 18;            // Norbi: 18 (dec) -> 0x12 (hex)
  int8_t  power       = 5 ;
  uint16_t preambleLength = 8;          // Norbi: 8
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
  bool stateLoRa = false;
  bool stateSD = false;
  bool statePredict = true;
  PacketInfo lastPacketInfo;
  ModemInfo modeminfo;
};
