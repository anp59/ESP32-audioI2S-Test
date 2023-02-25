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

// for directory listing over Serial (optional)
void listDir(fs::FS &fs, const char * dirname, int level = 0);
const char *name(File& f);
void printTabs(int level);
int numberOfFiles = 0;
unsigned int tabSize = 4; 

void setup()
{
    Serial.begin(115200);
    while (!Serial);
    
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(5); // 0...21

#if SDFATFS_USED
    // SPI settings here are not needed for SdFat
    Serial.printf("\nFile sytem 'SdFat' is used!\n");
    if (!SD.begin(SD_CS))   // SPI speed = 16 MHz (see audio.h)
        SD.initErrorHalt(); // SdFat-lib helper function

    // 64: EXFAT, 32: FAT32, 16: FAT16, or zero for error
    Serial.printf("FAT-Type is %d\n", SD.fatType());
    
    // list directory structure 
    //SD.ls("/", LS_SIZE | LS_R);
#else
    Serial.printf("\nFile sytem 'FS' is used!\n");
    pinMode(SD_CS, OUTPUT);      digitalWrite(SD_CS, HIGH);
    SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
    SPI.setFrequency(1000000);
    SD.begin(SD_CS);

    
#endif
    listDir(SD, "/"); 
    Serial.printf("\nTotal number of files = %d\n\n", numberOfFiles);
    /* 
    The data type "File32" can also be used. The type of "File" depends on SDFAT_FILE_TYPE of the SdFat library.
    ESP32-audioI2S currently only works with SDFAT_FILE_TYPE = 1, i.e. FAT16 and FAT32. 
    extFAT (SDFAT_FILE_TYPE = 2 or 3) would require an adaptation in ESP32-audioI2S 
    (e.g. variables for file sizes etc. would have to be adapted to the uint64_t data type.
    */
    File file;        
    file = SD.open("/test.mp3");
    if ( file ) {
        Serial.printf("file opened!\n");
        Serial.printf("file size: %d\n", file.size());
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


//###############################################################
// optional functions for listing directory

const char *name(File& f)
{
#ifdef SDFATFS_USED
    static char buf[256];
    buf[0] = 0;
    if ( f ) 
        f.getName(buf, sizeof(buf));
    return (const char *)buf;
#else
    return f.name();
#endif
}

void printTabs(int level)
{
    while ( level-- )
        for ( int i = 0; i < tabSize; i++)
             Serial.print(" ");
}

// list all files of dirname (also includes protected system files) 
void listDir(fs::FS &fs, const char * dirname, int level) {
    char path[256] = "";
    int len = 0;
    int mode = 2;
    File root;

    if (level == 0)
        numberOfFiles = 0;
    root = fs.open(dirname);
    //root.open(dirname);       // possible for SdFat  
    if ( !root ) {
        Serial.println("Failed to open directory");
        return;
    }
    if ( !root.isDirectory() ) {
    //if ( !root.isDir() ) {    // possible for SdFat
        Serial.println("Not a directory");
        return;
    }
    printTabs(level);
    Serial.println("--------------------------------------------------------------------");
    printTabs(level);
    Serial.printf("DIR: %s\n", dirname);
    while (true) {
       #ifdef SDFATFS_USED
        File file;
        file = root.openNextFile();
        //file.openNext(&root, O_RDONLY);       // possible for SdFat
       #else   
        File file = root.openNextFile();
       #endif
        while ( file ) {
            if ( file.isDirectory() && mode == 1 ) {
            //if ( file.isDir() && mode == 1 ) {    // possible for SdFat
                //Serial.printf("DIR: %s (L%d) \n", name(file), level);
                //if(level){
                    if ( (name(file))[0] != '/' ) {
                        strcpy(path, dirname); 
                        len = strlen(path);
                        if ( !(len == 1 && path[0] == '/') )    // not root (/) 
                            path[len++] = '/'; // ohne abschliessende 0
                    }
                    strcpy(path+len, name(file));
                    listDir(fs, path, level+1);
                //}
            } 
            if ( !file.isDirectory() && mode == 2 ) {
            //if ( !file.isDir() && mode == 2 ) {
                printTabs(level);
                Serial.printf(" FILE: %s (L%d)\n", name(file), level);
                numberOfFiles++;
            }
            file.close();
            file = root.openNextFile();
            //file.openNext(&root, O_RDONLY);   // possible for SdFat
        }
        mode--;
        if ( !mode ) {
            root.close();
            break;
        }
        root.rewindDirectory();
        // if ( root.isDir() ) root.rewind();   // possible for SdFat  
    }
}