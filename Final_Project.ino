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
#include <SerialFlash.h>
#include <Bounce.h>

/************  SETUP FOR RECORDING  ************/
AudioInputI2S            i2s2;           
AudioAnalyzePeak         peak1;     
AudioRecordQueue         queue1;    
AudioPlaySdRaw           playRaw1;  
AudioOutputI2S           i2s1;    
AudioConnection          patchCord1(i2s2, 0, queue1, 0);
AudioConnection          patchCord2(i2s2, 0, peak1, 0);
AudioConnection          patchCord3(playRaw1, 0, i2s1, 0);
AudioConnection          patchCord4(playRaw1, 0, i2s1, 1);
AudioControlSGTL5000     sgtl5000_1;

/************  GLOBALS & PIN ASSIGNMENTS  ************/
#define RST_PIN          37
#define SS_PIN           38
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14
const int myInput = AUDIO_INPUT_MIC;
const int BUTTON = 32;
File frec;                         //file where data is recorded
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance.
Bounce buttonRecord = Bounce(BUTTON, 8);
char *curr_tag = NULL;
bool recording = false;


/******************************************************************************************
 *                                        Setup:                                          *
 * ****************************************************************************************
*/
void setup() {
  pinMode(BUTTON, INPUT_PULLUP);

  AudioMemory(60);
  sgtl5000_1.enable();
  sgtl5000_1.inputSelect(myInput);
  sgtl5000_1.volume(0.5);

  Serial.begin(9600);
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  SPI.begin();
  Serial.println("Initializing Card...");
  mfrc522.PCD_Init();  // Init MFRC522 card
  mfrc522.PCD_DumpVersionToSerial(); //show details of card reader module
  Serial.println("Done.");


  Serial.println("Initializing SD Card...");
  if (!(SD.begin(SDCARD_CS_PIN))) {
    // stop here if no SD card, but print a message
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(1000);
    }
  }
  Serial.println("Done.");




  Serial.println("\n\n--Begin Main--\n\n");
}


/******************************************************************************************
 *                                      Main Loop:                                        *
 * ****************************************************************************************
*/
void loop() {
  buttonRecord.update();


  if (buttonRecord.fallingEdge()) {
    Serial.println(curr_tag);
    record();
  }


  else {
    if (mfrc522.PICC_IsNewCardPresent()) {
      free(curr_tag);
      Serial.println("Found Card!");

      //make sure we can read the card
      if ( !mfrc522.PICC_ReadCardSerial()) return;

      //construct the filename from the card's UID
      String file = "";

      for (byte i = 0; i < mfrc522.uid.size - 1; i++) {
        file += String(mfrc522.uid.uidByte[i]);
      }
      file += ".RAW";

      //some arguments for SD functions must be of type (char *) 
      //not String so we must convert the String to a Char*
      char *buf = (char*)malloc(sizeof(char) * (file.length() + 1));
      file.toCharArray(buf, file.length() + 1);
      curr_tag = buf;

      Serial.print("Tag Filename: ");
      Serial.println(curr_tag);
      play_msg(curr_tag);
    }
    mfrc522.PICC_HaltA();
  }
}


/******************************************************************************************
 *            record: while button is held, record from mic                               *
 *                         and write to SD card                                           *
 * ****************************************************************************************
*/
void record() {
  if (curr_tag == NULL) {
    Serial.println("ERROR: No Card has been tapped");
    return;
  }
  Serial.print("Recording to file ");
  Serial.print(curr_tag);
  Serial.println("...");
  startRecording(curr_tag);
  if (!recording) {
    Serial.println("ERROR: Unable to write to SD card");
    return;
  }
  while (!digitalRead(BUTTON)) {
    continueRecording();
  }
  stopRecording(curr_tag);
}


/******************************************************************************************
 *            play_msg: open sound file from SD card and playback                         *
 *                                                                                        *
 * ****************************************************************************************
*/
void play_msg(char *filename) {
  Serial.print("Playing Message from file ");
  Serial.print(filename);
  Serial.println("...");
  if (!SD.exists(filename)) {
    Serial.println("No recording for this tag yet");
    return;
  }
  playRaw1.play(filename);
  while (playRaw1.isPlaying()) {
  }
  playRaw1.stop();
  Serial.println("\nDone.");
}


/******************************************************************************************
 *            startRecording: open SD file and begin the recording queue                  *
 *                                                                                        *
 * ****************************************************************************************
*/
void startRecording(char *filename) {
  Serial.println("Recording...");
  if (SD.exists(filename)) {
    // The SD library writes new data to the end of the
    // file, so to start a new recording, the old file
    // must be deleted before new data is written.
    SD.remove(filename);
  }
  frec = SD.open(filename, FILE_WRITE);
  Serial.println(filename);
  if (frec) {
    queue1.begin();
    recording = true;
  }
}


/******************************************************************************************
 *            continueRecording: Fetch 2 blocks from the audio library and copy           *
 *                into a 512 byte buffer.  The Arduino SD library is most efficient       *
 *                when full 512 byte sector size writes are used.                         *
 * ****************************************************************************************
*/
void continueRecording() {
  if (queue1.available() >= 2) {
    byte buffer[512];
    memcpy(buffer, queue1.readBuffer(), 256);
    queue1.freeBuffer();
    memcpy(buffer + 256, queue1.readBuffer(), 256);
    queue1.freeBuffer();
    // write all 512 bytes to the SD card
    frec.write(buffer, 512);
  }
}


/******************************************************************************************
 *            stopRecording: stop recording to the queue, save additional data left       *
 *                           on the queue, and close the SD file                          *
 *                                                                                        *
 * ****************************************************************************************
*/
void stopRecording(char *filename) {
  queue1.end();
  if (recording) {
    //write the rest of the audio buffer to the file and close it
    while (queue1.available() > 0) {
      frec.write((byte*)queue1.readBuffer(), 256);
      queue1.freeBuffer();
    }
    frec.close();
  }
  recording = false;
  Serial.print("\nRecorded to ");
  Serial.println(filename);
}


