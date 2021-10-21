/************************************************************************************************
 *           Sketch for the communication between an Arduino and the Nucleo Board
 * 
 *
 *************************************************************************************************/
                                                         
                                  /* Test profibus byte */

const uint8_t DA = 0x00;
const uint8_t SA = 0x01;
const uint8_t FC = 0x02;
const uint8_t DSAP = 0x10;
const uint8_t SSAP = 0x20;
const uint8_t SD2 = 0x68;
const uint8_t ED = 0x16;

                                    /* Test payload */
const uint8_t payload1[] = {0x48, 0x65, 0x6C, 0x6C, 0x65, 0x77, 0x6F, 0x72, 0x6C, 0x64};
const uint8_t payload2[] = {0x45, 0x44, 0x55, 0x43, 0x41, 0x54};
const uint8_t payload3[] = {0x01, 0x02, 0x03, 0x04};

                                    /* Test variable */

uint8_t profibus_packet[255];
uint8_t half_packet[4];

void packet_build(uint8_t * packet, uint8_t * payload);
uint8_t calculate_FCS(uint8_t * payload);
uint8_t calculate_LE(uint8_t * payload);
void introduce_ERR(uint8_t * packet);
void print_profibus_packet(uint8_t * packet, uint8_t len);
void print_packet(uint8_t * packet, uint8_t len);

void setup()
{
  
      Serial.begin(115200);  
      
      packet_build(profibus_packet, payload3);
   
      for(uint8_t i = 0; i < 4; i++)
     {
        half_packet[i] = profibus_packet[i];
      }
     
      uint8_t second_half_packet[profibus_packet[1] + 2];

      for(uint8_t j = 0;  j < (profibus_packet[1] + 2); j++)
      {
        second_half_packet[j] = profibus_packet[4 + j]; 
     }
     
      print_profibus_packet(profibus_packet, profibus_packet[1]);
      delay(100);
      
      print_packet(half_packet, 4);
      delay(100);
      print_packet(second_half_packet, (profibus_packet[1] + 2));  
}

void loop()
{
}

                    /* Build of the profibus packet function */ 
void packet_build(uint8_t * packet, const uint8_t * payload)
{
  uint8_t tmp;
  
  packet[0] = SD2;
  packet[1] = calculate_LE(payload);
  packet[2] = calculate_LE(payload);
  packet[3] = SD2;
  packet[4] = DA;  
  packet[5] = SA;  
  packet[6] = FC;  
  packet[7] = DSAP;  
  packet[8] = SSAP;  
  
  for(uint8_t i = 0; payload[i] != '\0'; i++)
  {
    tmp = 8 + (i + 1);
    packet[tmp] = payload[i];
  }

  packet[tmp + 1] = calculate_FCS(payload);
  packet[tmp + 2] = ED;
}

                   /* Function for the calc of the FCS*/
            /* FCS = DA + SA + FC + DSAP + SSAP + DU */
uint8_t calculate_FCS(const uint8_t * payload) /*OK WORKING*/
{
  uint8_t fcs = 0;
  fcs = DA + SA + FC + DSAP + SSAP;
  
  for(uint8_t i = 0; payload[i] != '\0'; i++)
  {
    fcs += payload[i];
  }
  
  return fcs; 
  
}
                          /* Function for the calc of the length*/
                  /* LE  = Len of (DA + SA + FC + DSAP + SSAP + DU */
uint8_t calculate_LE(const uint8_t * payload) /* ok working */
{
  uint8_t len;
  len = 5;

  for(uint8_t i = 0; payload[i] != '\0'; i++)
  {

    len ++;
    
  }

  return len;
}

                        /* Print the profibus packet */
void print_profibus_packet(uint8_t * packet, uint8_t len) /*ok working */
{
  for (uint8_t i = 0; i < (len + 6); i++)
  {
    Serial.write(packet[i]);
  }
}

                         /* Print any kind of packet */
void print_packet(uint8_t *packet, uint8_t len)
{
  for(uint8_t i = 0; i < len ; i++)
  {
    Serial.write(packet[i]);
  }
}

                      /* Function to add some error on the packet  */ 
void introduce_ERR(uint8_t * packet)
{
  /* Introduce error in the FCS */
  
  packet[packet[1] + 4] = 0xFF;
  
}




