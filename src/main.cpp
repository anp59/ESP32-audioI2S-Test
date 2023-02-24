//**********************************************************************************************************
//*    audioI2S-- I2S audiodecoder for ESP32,  SdFat example   
//*    !!!  For the SdFat example or for using the SdFat library with ESP32-audioI2S, an adapted version 
//*    !!!  of this library is used: https://github.com/anp59/ESP32-audioI2S.git                                                           *
//**********************************************************************************************************
//
// first release on 05/2020
// updated on Feb. 24, 2023
/*

1. install SdFat V2 from https://github.com/greiman/SdFat or over lib_deps in platformio.ini
2. activate "SDFATFS_USED"                   in Audio.h
3. activate "#define USE_UTF8_LONG_NAMES 1"  in SdFatConfig.h or in platformio.ini
*/

#include "Arduino.h"
#include "Audio.h"
#include "SPI.h" // not needed with SdFat

// Digital I/O used
#define SD_CS 5
#define SPI_MOSI 23
#define SPI_MISO 19
#define SPI_SCK 18
#define I2S_DOUT 25
#define I2S_BCLK 27
#define I2S_LRC 26

Audio audio;

void setup()
{
    // SPI settings here are not needed for SdFat:
    // pinMode(SD_CS, OUTPUT);      digitalWrite(SD_CS, HIGH);
    // SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    // SPI.setFrequency(1000000);
    
    Serial.begin(115200);
    while (!Serial);
    
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(5); // 0...21

    if (!SD.begin(SD_CS))   // SPI speed = 16 MHz (see audio.h)
        SD.initErrorHalt(); // SdFat-lib helper function
    
    // 64: EXFAT, 32: FAT32, 16: FAT16, or zero for error
    Serial.printf("\nFAT-Type is %d\n", SD.fatType());
    
    /* 
    The data type "File32" can also be used. The type of "File" depends on SDFAT_FILE_TYPE of the SdFat library.
    ESP32-audioI2S currently only works with SDFAT_FILE_TYPE = 1, i.e. FAT16 and FAT32. 
    extFAT (SDFAT_FILE_TYPE = 2 or 3) would require an adaptation in ESP32-audioI2S 
    (e.g. variables for file sizes etc. would have to be adapted to the uint64_t data type.
    */
    File file;        
    file.open("/test.mp3");
    if ( file ) {
        Serial.println("file opened!");
        Serial.printf("file size: %d, directory index: %d\n", file.fileSize(), file.dirIndex());
        file.close();
    } 
    else  
        Serial.println("file not exists!");

    audio.connecttoFS(SD, "/test.mp3");
    //audio.connecttoFS(SD, "良い一日私の友達.mp3");
}

void loop()
{
    audio.loop();
}

// optional
void audio_info(const char *info)
{
    Serial.print("info        ");
    Serial.println(info);
}
void audio_id3data(const char *info)
{ // id3 metadata
    Serial.print("id3data     ");
    Serial.println(info);
}
void audio_eof_mp3(const char *info)
{ // end of file
    Serial.print("eof_mp3     ");
    Serial.println(info);
}
