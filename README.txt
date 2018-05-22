**** 60-223 Physical Computing Final Project: Assistive Device ****
                             Bookmark Box
			
		    Logan Kojiro -- Kristie Lord		


      Process & Physical Documentation: 
      https://courses.ideate.cmu.edu/60-223/s2018/work/book-reminder-by-dog-loving-duo-final-documentation/

*******************************************************************
		            Overview:  

The goal of this project was to create an 'assistive device' for an 
assigned client from a community of older people.  This device 
should make some part of the client's life easier, more interesting,
or enable them to do something they couldn't do otherwise.

Our client, Joseph, is an avid reader.  He and his wife are in 
multiple book clubs and between them will usually be reading
at least 6 books concurrently.  To make keeping track of what
is happening in each of these books easier, we envisioned
a device that would allow them to store a brief summary of the recent
events and plot points whenever they put the book down.

Using bookmarks with RFID tags built into them, each bookmark identifies
a unique recording file which stores a summary of an arbitrary length
(limited only by the size of the micro SD card in the system).  Tapping
the bookmark to the device will trigger the playback of the most recent 
recording, and then allows the user to record a new message to the file.

*******************************************************************
		   Noteable Hardware/Libraries:

RFID Reader: MFRC522 https://playground.arduino.cc/Learning/MFRC522
	Used to read from bookmark to turn tag's UID into a filename

Audio Playback/Recording: Audio Adaptor Board for Teensy 3.0-3.6
			  + Teensy audio Library, AudioPlaySdRaw
			  https://www.pjrc.com/store/teensy3_audio.html
			  https://www.pjrc.com/teensy/td_libs_AudioPlaySdRaw.html
	Chip connects to SD card and enables playback to external
	speakers and recording from built in mic.
 
