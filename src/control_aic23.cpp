#include <Arduino.h>
#include <Wire.h>
#include "control_aic23.h"

bool AudioControlAIC23::enable(void){
    Wire.begin();
    //Serial.begin(9600);
    aic23_init();
    return true;
}
/* Recommended startup sequence:
    https://bit.ly/3pYIajW
    reset register (Reg. 0x0F)
    set your clock/sample rate (Reg. 0x08)
    set audio format (Reg. 0x07)
    activate digital interface (Reg. 0x09)
    set input/output volume (Reg. 0x00 to Reg. 0x03)
    set analog path (Reg. 0x04)
    set digital path if needed (Reg. 0x05)
    enable the power (Reg. 0x06)
*/
void AudioControlAIC23::aic23_init(){
    reset();                  
    //sampleRateControl(); //Currently does not need to be changed from default (I think)                       
    //mute(false);                
    digitalAudioFormat(); // Configure for i2s (need to look up exact teensy audio format)        
    write(DIGT_ACT_CTRL, DIGT_ACT_ON);
    write(LINVOL_CTRL, DEFAULT_VOL_CTRL); //unmute, set L/R simultaneous update, default volume
    write(RINVOL_CTRL, DEFAULT_VOL_CTRL);
    write(ANLG_CTRL, DEFAULT_ANLG_CTRL); //Select DAC, Disable bypass 
    write(DIGT_CTRL, DEFAULT_DIGT_CTRL | TLV320AIC23_ADCHP_ON);  //un-soft mute DAC
    power(MIC_OFF);
}
/* Left line input channel volume control (Address: 0000000)
BIT      D8  D7  D6 D5 D4   D3   D2   D1   D0
Function LRS LIM X  X  LIV4 LIV3 LIV2 LIV1 LIV0
Default  0   1   0  0  1    0    1    1    1
LRS Left/right line simultaneous volume/mute update
    Simultaneous update 0 = Disabled 1 = Enabled
LIM Left line input mute 0 = Normal 1 = Muted
LIV[4:0] Left line input volume control (10111 = 0 dB default)
    11111 = +12 dB down to 00000 = –34.5 dB in 1.5-dB steps
X Reserved */
bool AudioControlAIC23::inputLevel(float volume){
    uint8_t vol = 0;
    vol = (uint8_t)(volume * 32.0);
    write(LINVOL_CTRL, (DEFAULT_VOL_CTRL & 0x1E0) | vol);
    //write(RINVOL_CTRL, (DEFAULT_VOL_CTRL & 0x1E0) | vol);
    return true;
}  
/* Analog Audio Path Control (Address: 0000100)
BIT      D8   D7   D6   D5  D4  D3  D2    D1   D0
Function STA2 STA1 STA0 STE DAC BYP INSEL MICM MICB
Default  0    0    0    0   0   1   0     1    0
STA[2:0] and STE:
    STE STA2 STA1 STA0 ADDED-SIDETONE
    1   1    X    X    0 dB
    1   0    0    0    −6 dB
    1   0    0    1    −9 dB
    1   0    1    0    −12 dB
    1   0    1    1    −18 dB
    0   X    X    X    Disabled
DAC DAC select 0 = DAC off 1 = DAC selected
BYP Bypass 0 = Disabled 1 = Enabled
INSEL Input select for ADC 0 = Line 1 = Microphone
MICM Microphone mute 0 = Normal 1 = Muted
MICB Microphone boost 0=dB 1 = 20dB
X Reserved */
void AudioControlAIC23::reset(void){
    int s;
    s = write(RESET_CTRL, 0x0);
    Serial.print(s);
}

void AudioControlAIC23::power(int pdc){
    int s;
    s = write(POWER_CTRL, pdc);
    Serial.print(s);
}

void AudioControlAIC23::digitalAudioFormat(void){
    int cmd = TLV320AIC23_MS_SLAVE | TLV320AIC23_IWL_16 | TLV320AIC23_FOR_I2S; // Slave Mode, 16 bit, i2s
    int s;
    s = write(DIGT_FMT_CTRL, cmd);
    Serial.print(s);
}

/* 
    Sample Rate Control (Address: 0001000)
    BIT:      D8 D7     D6    D5  D4  D3  D2  D1   D0
    Function: X  CLKOUT CLKIN SR3 SR2 SR1 SR0 BOSR USB/Normal
    Default:  0  0      0     1   0   0   0   0    0
    CLKOUT: Clock output divider 0 = MCLK 1 = MCLK/2
    CLKIN: Clock input divider 0 = MCLK 1 = MCLK/2
    SR[3:0]: Sampling rate control (see Sections 3.3.2.1 and 3.3.2.2) 
        (44.1kHz -> Default w/ MCLK = 11.2896 MHz)
    BOSR Base oversampling rate
        USB mode:    0 = 250 f s 1 = 272 f s
        Normal mode: 0 = 256 f s 1 = 384 f s
    USB/Normal: Clock mode select: 0 = Normal 1 = USB
    X            Reserved 
*/
// void AudioControlAIC23::sampleRateControl(void){
//     Wire.write();
//}

bool AudioControlAIC23::write(unsigned int reg, unsigned int val)
{
    byte data[2];
	Wire.beginTransmission(I2C_ADDRESS);
    data[0] = (reg<<1) | ((val >> 8) & 0x1);
    data[1] = val & 0xFF;
    Wire.write(data[0]);
    Wire.write(data[1]);
	return Wire.endTransmission() == 0;
}
