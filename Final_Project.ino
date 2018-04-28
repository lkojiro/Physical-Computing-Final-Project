/******************************************************************************************
 *                                                                                        *
 *                        Intro to Physical Computing: Final Project                      *
 *                                     Bookmark Box                                       *
 *                                                                                        *
 *                           Logan Kojiro  --  Kristie Lord                               *
 * ****************************************************************************************
 *              Parts of this are modified from a MFRC522 library example; see            *
 *       https://github.com/miguelbalboa/rfid for further details and other examples.     *
 *                                                                                        *
 *         Audio Recording is modified from example code with the teensy audio library    *
 *              See Examples->Audio->Recorder                                             *
 *                                                                                        *  
 *                              Released into the public domain.                          *     
 * ****************************************************************************************
 */


/************  LIBRARIES  ************/
#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>
#include <SD.h>
#include <Audio.h>
#include <play_sd_raw.h>
#include <play_sd_wav.h>
#include <SerialFlash.h>

/************  STUFF FOR THE RECORDING  ************/
AudioInputI2S            i2s2;           //xy=105,63
AudioAnalyzePeak         peak1;          //xy=278,108
AudioRecordQueue         queue1;         //xy=281,63
AudioPlaySdRaw           playRaw1;       //xy=302,157
AudioOutputI2S           i2s1;           //xy=470,120
AudioConnection          patchCord1(i2s2, 0, queue1, 0);
AudioConnection          patchCord2(i2s2, 0, peak1, 0);
AudioConnection          patchCord3(playRaw1, 0, i2s1, 0);
AudioConnection          patchCord4(playRaw1, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;     //xy=265,212

/************  GLOBALS & PIN ASSIGNMENTS  ************/
char *curr_tag;
AudioPlaySdRaw message;
#define RST_PIN         37
#define SS_PIN          38
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance.
File frec;                         //file where data is recorded
const int myInput = AUDIO_INPUT_LINEIN;
const int BUTTON = 4;

/************  FORWARD DECLARATIONS FOR HELPERS  ************/
void record(char *filename);
void play_msg(char *filename);



/******************************************************************************************
 *                                        Setup:                                          *
 * ****************************************************************************************
 */
void setup() {
  Serial.begin(9600);
  SPI.begin();         // Init SPI bus
  SPI.setMOSI(28);
  SPI.setMISO(39);
  SPI.setSCK(27);
  mfrc522.PCD_Init();  // Init MFRC522 card
  
}


/******************************************************************************************
 *                                      Main Loop:                                        *
 * ****************************************************************************************
 */
void loop() {
  bool rec = digitalRead(BUTTON);

  if (rec){
    record(curr_tag);
  }

  else {
      if (mfrc522.PICC_IsNewCardPresent()){
            
            
            ////////////////////////////
            // Select one of the cards
            ////////////////////////////
            if ( !mfrc522.PICC_ReadCardSerial()) return;

            char file[4];
            
            for (byte i = 0; i < mfrc522.uid.size; i++){
              file[i] = (mfrc522.uid.uidByte[i]);
            }
            
            Serial.print("filename: '");
            Serial.print(file);
            Serial.println(".wav'");
            curr_tag = file;
            play_msg(curr_tag);
            }
            mfrc522.PICC_HaltA();
  }
}


/******************************************************************************************
 *            record: while button is held, record from mic                               *
 *                        on release, write recording to SD card's memory                 *
 * ****************************************************************************************
 */
void record(char *filename){
  (void)filename;
}


/******************************************************************************************
 *            play_msg: open sound file from SD card and playback                         *
 *                                                                                        *
 * ****************************************************************************************
 */
void play_msg(char *filename){
  return;
  message.play(filename);
  while (message.isPlaying()){
    Serial.println(message.positionMillis());
  }
}



