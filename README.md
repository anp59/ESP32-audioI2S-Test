# ESP32-audioI2S-Test
ESP32-audioI2S lib test with SdFat 2.0

The use of the SdFat library speeds up the reading of the SD directory structure compared to the FS library and allows the use of additional functions in dealing with files and file structures.

In order to be able to use SdFat together with the popular library https://github.com/schreibfaul1/ESP32-audioI2S, various adjustments were made.
You can find the adapted version under https://github.com/anp59/ESP32-audioI2S.

Please note the comments in audio.h and in main.cpp.
