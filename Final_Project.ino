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
#include <SPI.h>
#include <MFRC522.h>
#include <SD.h>
#include <Audio.h>
#include <play_sd_raw.h>
#include <SerialFlash.h>
#include <Bounce.h>

/************  SETUP FOR RECORDING  ************/
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
#define RST_PIN          37
#define SS_PIN           38
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_MISO_PIN  8
#define SDCARD_SCK_PIN   14
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance.
File frec;                         //file where data is recorded
const int myInput = AUDIO_INPUT_LINEIN;
const int BUTTON = 32;
Bounce buttonRecord = Bounce(BUTTON, 8);
bool recording = false;

/************  FORWARD DECLARATIONS FOR HELPERS  ************/
void record(char *filename);
void play_msg(char *filename);
void startRecording(char *filename);
void continueRecording();
void stopRecording(char *filename);


/******************************************************************************************
 *                                        Setup:                                          *
 * ****************************************************************************************
 */
void setup() {
  pinMode(BUTTON, INPUT_PULLUP);
  Serial.begin(9600);
  SPI.begin();         // Init SPI bus
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setMISO(SDCARD_MISO_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  Serial.println("hang");
  mfrc522.PCD_Init();  // Init MFRC522 card
  
  AudioMemory(60);
  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(myInput);
  sgtl5000_1.volume(0.5);
  
  if (!(SD.begin(SDCARD_CS_PIN))) {
    // stop here if no SD card, but print a message
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }
}


/******************************************************************************************
 *                                      Main Loop:                                        *
 * ****************************************************************************************
 */
void loop() {
  buttonRecord.update();
  
  
  if (buttonRecord.risingEdge()){
    Serial.println("Recording");
    record(curr_tag);
  }


  else {
      if (mfrc522.PICC_IsNewCardPresent()){
            
            // Select one of the cards
            if ( !mfrc522.PICC_ReadCardSerial()) return;

            char file[mfrc522.uid.size];
            
            for (byte i = 0; i < mfrc522.uid.size; i++){
              file[i] = (mfrc522.uid.uidByte[i]);
            }
            
            curr_tag = strcat(file,".RAW");
            Serial.print("New tag: ");
            Serial.println(curr_tag);
            play_msg(curr_tag);
            }
            mfrc522.PICC_HaltA();
  }
}


/******************************************************************************************
 *            record: while button is held, record from mic                               *
 *                        and write to SD card                                            *
 * ****************************************************************************************
 */
void record(char *filename){
  startRecording(filename);
  if (!recording){
    Serial.println("Unable to write to SD card");
    return;
  }
  while(digitalRead(BUTTON)){
    Serial.print(".");
    continueRecording();
  }
  stopRecording(filename);
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
  message.stop();
}


/******************************************************************************************
 *            startRecording: open SD file 'filename' and begin the recording queue       *
 *                                                                                        *
 * ****************************************************************************************
 */
void startRecording(char *filename) {
  if (SD.exists(filename)) {
    // The SD library writes new data to the end of the
    // file, so to start a new recording, the old file
    // must be deleted before new data is written.
    SD.remove(filename);
  }
  frec = SD.open(filename, FILE_WRITE);
  if (frec) {
    Serial.println("Recording...");
    queue1.begin();
    recording = true;
  }
}


/******************************************************************************************
 *            continueRecording: Fetch 2 blocks from the audio library and copy           *
 *               into a 512 byte buffer.  The Arduino SD library is most efficient        *
 *               when full 512 byte sector size writes are used.                          *
 * ****************************************************************************************
 */
void continueRecording() {
  if (queue1.available() >= 2) {
    byte buffer[512];
    memcpy(buffer, queue1.readBuffer(), 256);
    queue1.freeBuffer();
    memcpy(buffer+256, queue1.readBuffer(), 256);
    queue1.freeBuffer();
    // write all 512 bytes to the SD card
    frec.write(buffer, 512);
  }
}


/******************************************************************************************
 *            stopRecording: end the recording queue and close the SD file                *
 *                                                                                        *
 * ****************************************************************************************
 */
void stopRecording(char *filename) {
  Serial.print("Recorded to ");
  Serial.println(filename);
  queue1.end();
  if (recording) {
    while (queue1.available() > 0) {
      frec.write((byte*)queue1.readBuffer(), 256);
      queue1.freeBuffer();
    }
    frec.close();
  }
  recording = false;
}


