#include <Audio.h>
#include <stdio.h>
#include <IntervalTimer.h>
#include <Encoder.h>
#include <Bounce.h>
#include <LiquidCrystal_I2C.h> //YWROBOT
#include "Menus.h"
#include "control_aic23.h"

#define AUDIO_MASK 0x0000FFFF
#define NUM_BLOCKS 256
#define CIRCULAR_BUFFER_LENGTH AUDIO_BLOCK_SAMPLES * NUM_BLOCKS
#define CIRCULAR_BUFFER_MASK (CIRCULAR_BUFFER_LENGTH - 1)
#define MAX_SAMPLE_BLOCKS NUM_BLOCKS - 1
//Audio library macro, AUDIO_BLOCK_SAMPLES = 128
#define BUTTON 6
#define ENC_BUTTON 5
#define ENC_A 3
#define ENC_B 4
#define DC_BIAS 1150 // measured value
#define LED 15

AudioInputI2S i2s_in;
AudioOutputI2S i2s_out;
AudioRecordQueue Q_in_L;
AudioRecordQueue Q_in_R;
AudioPlayQueue Q_out_L;
AudioPlayQueue Q_out_R;
AudioConnection patchCord1(i2s_in, 0, Q_in_L, 0);
AudioConnection patchCord2(i2s_in, 1, Q_in_R, 0);
AudioConnection patchCord3(Q_out_L, 0, i2s_out, 0);
AudioConnection patchCord4(Q_out_R, 0, i2s_out, 1);
AudioControlAIC23 aic23;
IntervalTimer audioProcesser;
LiquidCrystal_I2C lcd(0x27,20,4); // set the LCD address to 0x27

Encoder enc1(ENC_A, ENC_B);
Bounce encBtn = Bounce(ENC_BUTTON, 5); // Instantiate a Bounce object with a 5 millisecond debounce time
Bounce btn = Bounce(BUTTON, 10); // Instantiate a Bounce object with a 5 millisecond debounce time

#define NUMELEMENTS(X) (sizeof(X) / sizeof(X[0]))
MENU *currentMenu = &mainMenu;            // currently selected menu
bool (*currentAction)(void *ptr) = NULL;  // function to execute
void *currentParam = NULL;                // parameter for function to execute

volatile bool updateLCD = false;
volatile bool btnFlag = false;
volatile uint16_t count = 0;
volatile uint16_t processCount= 0;
float a_coeff[4] = {0};
float b_coeff[4] = {0};
uint16_t prevHigh=0, prevMid=0, prevLow=0;
float leftIn[CIRCULAR_BUFFER_LENGTH] = {0.0};
float rightIn[CIRCULAR_BUFFER_LENGTH] = {0.0};
float leftOut[AUDIO_BLOCK_SAMPLES] = {0.0};
float rightOut[AUDIO_BLOCK_SAMPLES] = {0.0};
int16_t *bp_L, *bp_R;
uint16_t curBlock = 0;
uint32_t curSample = 0;
uint32_t startIndex = 0;
uint32_t outputIndex = 0;
float32_t vtime = 0;
uint8_t toggle = 0;

void audioProcess(){    
    bp_L = Q_in_L.readBuffer();
    bp_R = Q_in_R.readBuffer();

    startIndex = curBlock * AUDIO_BLOCK_SAMPLES;
    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
      curSample = startIndex + i;
      leftIn[curSample] = (float)(bp_L[i] + DC_BIAS);
      //rightIn[curSample] = (float)(bp_R[i] + DC_BIAS);
    }

    Q_in_L.freeBuffer();
    Q_in_R.freeBuffer();

    bp_L = Q_out_L.getBuffer();
    bp_R = Q_out_R.getBuffer();

    for (outputIndex = 0; outputIndex < AUDIO_BLOCK_SAMPLES; outputIndex++) {
      
      curSample = startIndex + outputIndex;
      leftOut[outputIndex] = 0;
      rightOut[outputIndex] = 0;

      switch(*curEffect)
      {
        case equalizer:
          audioEquilizer();
          break;
        case echo:
          audioEcho();
          break;
        case distortion:
          audioDistortion();
          break;
        case fuzz:
          audioFuzz();
          break;
        case octaver:
          audioOctaver();
          break;
        case flanger:
          audioFlanger();
          break;
        case tremelo:
          audioTremelo();
          break;
        case binarize:
          audioBinarize();
          break;
        default:
          audioPassThrough();
      }
    }
    for (int i = 0; i < AUDIO_BLOCK_SAMPLES; i++){
      //Serial.println(leftOut[i]);
      bp_L[i] = (int16_t)leftOut[i];
      // bp_R[i] = (int16_t)rightOut[i];
      bp_R[i] = (int16_t)leftOut[i];
    }

    curBlock++;
    curBlock &= MAX_SAMPLE_BLOCKS;

    // and play them back into the audio queues
    Q_out_L.playBuffer();
    Q_out_R.playBuffer();
}

void audioBinarize(){
  if(leftIn[curSample] > 10) leftOut[outputIndex] = 3000;
  else if(leftIn[curSample] < -10) leftOut[outputIndex] = -3300;
  else leftOut[outputIndex] = leftIn[curSample];
}
void audioMovingAverage(){
  int filLen = 50;
  int amplify = 3;

  for (int j = 0; j < filLen; ++j)
  {
    leftOut[outputIndex] += leftIn[(curSample - j) & CIRCULAR_BUFFER_MASK];
    rightOut[outputIndex] += rightIn[(curSample - j) & CIRCULAR_BUFFER_MASK];
  }
  leftOut[outputIndex] = amplify * (leftOut[curSample] / filLen);
  rightOut[outputIndex] = amplify * (rightOut[curSample] / filLen);
}

void audioEcho(){
  //parameters
  float feedback = curFeedback / 100.0; 
  float tempRight=0.0, tempLeft=0.0;
  int numEchos = 30, i=0; 
  unsigned int delays = curDelay;
  
  while(i < numEchos){
    tempLeft += feedback*leftIn[(curSample-delays) & CIRCULAR_BUFFER_MASK];
    tempRight += feedback*rightIn[(curSample-delays) & CIRCULAR_BUFFER_MASK];
    delays += curDelay;
    feedback -= feedback / 2;
    i++;
  }

  leftOut[outputIndex] = leftIn[curSample] + tempLeft;
  rightOut[outputIndex] = rightIn[curSample] + tempRight;   
}

void audioDistortion(){
  float sign = (leftIn[curSample] > 0) - (leftIn[curSample] < 0);
  float sum = 1.0;
  float cur = 1.0;
  float bottom = 1.0;
  int order=50;
  //parameters
  float gain= curGain;
  float boost= curBoost;
  //Normalize signed 16 bit signal 
  leftIn[curSample] /= 32768.0;
  float temp = leftIn[curSample] * gain * boost;
  // Appoximate function for sign*[1-exp(sign*gain*input)]
  if(sign < 0){
    if(curAsym == 1 || curAsym == 0){  
      for (int j = 1; j < order; ++j){
        cur *= temp;
        bottom *= j;
        cur /= bottom;
        sum += cur;
      }
      leftOut[outputIndex] = sum - 1;
      //Limit clipping on output
      if(leftOut[outputIndex] > 1.0) leftOut[outputIndex] = 1.0;
      //leftOut[outputIndex] /= gain;
      leftOut[outputIndex] *= 32768.0;
    }
    else{
      leftOut[outputIndex] = leftIn[curSample];
    }
  }
  else{
    if(curAsym == 2 || curAsym == 0){  
      temp *= -1;
      for (int j = 1; j < order; ++j){
        cur *= temp;
        bottom *= j;
        cur /= bottom;
        sum += cur;
      }
      leftOut[outputIndex] = 1 - sum;
      //Limit clipping on output
      if(leftOut[outputIndex] < -1.0) leftOut[outputIndex] = -1.0;
      //leftOut[outputIndex] /= gain;
      leftOut[outputIndex] *= 32768.0;
    }
    else{
      leftOut[outputIndex] = leftIn[curSample];
    }
  }
  rightOut[outputIndex] = leftOut[outputIndex];
}

void audioFuzz(){
  //parameters
  int limit = curLimit;
  int filLen = curFiltLen;
  int gain = curFuzzGain;
  
  float rightTemp =0.0, leftTemp=0.0;
 
  for (int j = 0; j < filLen; ++j)
  {
    leftTemp += leftIn[(curSample - j) & CIRCULAR_BUFFER_MASK];
    rightTemp += rightIn[(curSample - j) & CIRCULAR_BUFFER_MASK];
  }

  leftTemp = gain * (leftTemp / filLen);
  rightTemp = gain * (rightTemp /  filLen);

  if(rightTemp > limit) rightOut[outputIndex] = limit;
  else if(rightTemp < -limit) rightOut[outputIndex] = -limit;
  else rightOut[outputIndex]=rightTemp;

  if(leftTemp > limit) leftOut[outputIndex] = limit;
  else if(leftTemp < -limit)leftOut[outputIndex] = -limit;
  else leftOut[outputIndex]=leftTemp;
 
}

void audioFlanger(){
  float32_t freq = curSpeed; // 1 - 11 Hz
  float32_t mod = arm_sin_f32(2*PI*freq*vtime);
  float32_t depth = curDepth; //samples between 0 - 2 ms or about 0 - 88 
  vtime += 1 / 44100.0;
  if(vtime >= (float32_t)(1 / freq)) vtime = 0;
  uint16_t delay = (uint16_t)(depth+depth*mod);
  leftOut[outputIndex] = leftIn[curSample] + leftIn[(curSample-delay) & CIRCULAR_BUFFER_MASK] ;
  rightOut[outputIndex] = rightIn[curSample] + rightIn[(curSample-delay) & CIRCULAR_BUFFER_MASK];
}

void audioTremelo(){
  float32_t freq = curTremSpeed;
  float32_t depth = curTremDepth / 100.0;
  float32_t wave = 0.8;
  float32_t squareMod = (vtime * 44100) < (freq / 2) ? 1 : 0;
  float32_t sinMod = arm_sin_f32(2*PI*freq*vtime) + 1;
  vtime += 1 / 44100.0;
  if(vtime >= (float32_t)(1 / freq)) vtime = 0;
  
  leftOut[outputIndex] = leftIn[curSample] * (1-depth) + leftIn[curSample] * ((sinMod*wave) + (squareMod*(1-wave))) * depth;
  rightOut[outputIndex] = rightIn[curSample] * (1-depth) + rightIn[curSample] * ((sinMod*wave) + (squareMod*(1-wave))) * depth;
}

void audioOctaver(){
  float feedback = 0.5;
  leftOut[outputIndex] = leftIn[curSample] + abs(leftIn[curSample])*feedback;
  rightOut[outputIndex] = rightIn[curSample] + abs(rightIn[curSample])*feedback;
}

void audioEquilizer(){
  if(curHigh != prevHigh || curMid != prevMid || curLow != prevLow){
    float low = curLow / 100.0;
    float mid = curMid / 100.0;
    float high = curHigh / 100.0;

    updateBassmanCoeff(low, mid, high);

    prevHigh = curHigh;
    prevMid = curMid;
    prevLow = curLow;
  }
  
  //Type II direct implementation of IIR filter
  for (int j = 1; j <= 3; j++)
    {
      leftIn[curSample] -= (float)(a_coeff[j] * leftIn[(curSample - j) & CIRCULAR_BUFFER_MASK]);
      rightIn[curSample] -= (float)(a_coeff[j] * rightIn[(curSample - j) & CIRCULAR_BUFFER_MASK]);
    }

  for (int j = 0; j <= 3; j++)
  {
    leftOut[outputIndex] += (float)(b_coeff[j] * leftIn[(curSample - j) & CIRCULAR_BUFFER_MASK]);
    rightOut[outputIndex] += (float)(b_coeff[j] * rightIn[(curSample - j) & CIRCULAR_BUFFER_MASK]);
  }
}

void updateBassmanCoeff(float low, float mid, float top){
  // Parameters:
  // low = bass level
  // mid = noise level
  // top = treble level
  float fs = 44100.0; // Sample rate
  float C1 = 0.25e-9; // analog compenent value
  float C2 = 20.0e-9;
  float C3 = 20.0e-9;
  float R1 = 250.0e3; 
  float R2 = 1.0e6; 
  float R3 = 25.0e3;
  float R4 = 56.0e3;

  // Analog transfer function coefficients:
  double b1 = top*C1*R1 + mid*C3*R3 + low*(C1*R2 + C2*R2) + (C1*R3 + C2*R3);
  double b2 = top*(C1*C2*R1*R4 + C1*C3*R1*R4) - mid*mid*(C1*C3*R3*R3 + C2*C3*R3*R3)
            + mid*(C1*C3*R1*R3 + C1*C3*R3*R3 + C2*C3*R3*R3)
            + low*(C1*C2*R1*R2 + C1*C2*R2*R4 + C1*C3*R2*R4)
            + low*mid*(C1*C3*R2*R3 + C2*C3*R2*R3);
            + (C1*C2*R1*R3 + C1*C2*R3*R4 + C1*C3*R3*R4);
  double b3 = low*mid*(C1*C2*C3*R1*R2*R3 + C1*C2*C3*R2*R3*R4)
            - mid*mid*(C1*C2*C3*R1*R3*R3 + C1*C2*C3*R3*R3*R4)
            + mid*(C1*C2*C3*R1*R3*R3 + C1*C2*C3*R3*R3*R4)
            + top*C1*C2*C3*R1*R3*R4 - top*mid*C1*C2*C3*R1*R3*R4
            + top*low*C1*C2*C3*R1*R2*R4;
  double a0 = 1;
  double a1 = (C1*R1 + C1*R3 + C2*R3 + C2*R4 + C3*R4) 
              + mid*C3*R3 + low*(C1*R2 + C2*R2);
  double a2 = mid*(C1*C3*R1*R3 - C2*C3*R3*R4 + C1*C3*R3*R3 + C2*C3*R3*R3)
            + low*mid*(C1*C3*R2*R3 + C2*C3*R2*R3)
            - mid*mid*(C1*C3*R3*R3 + C2*C3*R3*R3)
            + low*(C1*C2*R2*R4 + C1*C2*R1*R2 + C1*C3*R2*R4 + C2*C3*R2*R4)
            + (C1*C2*R1*R4 + C1*C3*R1*R4 + C1*C2*R3*R4 + C1*C2*R1*R3
            + C1*C3*R3*R4 + C2*C3*R3*R4);
  double a3 = low*mid*(C1*C2*C3*R1*R2*R3 + C1*C2*C3*R2*R3*R4)
            - mid*mid*(C1*C2*C3*R1*R3*R3 + C1*C2*C3*R3*R3*R4)
            + mid*(C1*C2*C3*R3*R3*R4 + C1*C2*C3*R1*R3*R3 - C1*C2*C3*R1*R3*R4)
            + low*C1*C2*C3*R1*R2*R4 + C1*C2*C3*R1*R3*R4;

  // Digital filter coefficients:
  double c = 2*fs;
  double a0_temp = -a0 - a1*c - a2*c*c - a3*c*c*c;
  double a1_temp = (-3*a0 - a1*c + a2*c*c + 3*a3*c*c*c) / a0_temp;
  double a2_temp = (-3*a0 + a1*c + a2*c*c - 3*a3*c*c*c) / a0_temp;
  double a3_temp = (-a0 + a1*c - a2*c*c + a3*c*c*c) / a0_temp;
  double b0_temp = (-b1*c - b2*c*c - b3*c*c*c) / a0_temp; 
  double b1_temp = (-b1*c + b2*c*c + 3*b3*c*c*c) / a0_temp;
  double b2_temp = (b1*c + b2*c*c - 3*b3*c*c*c) / a0_temp;
  double b3_temp = (b1*c - b2*c*c + b3*c*c*c) / a0_temp;

  a_coeff[0] = 1.0;
  a_coeff[1] = (float) a1_temp;
  a_coeff[2] = (float) a2_temp;
  a_coeff[3] = (float) a3_temp;
  b_coeff[0] = (float) b0_temp;
  b_coeff[1] = (float) b1_temp;
  b_coeff[2] = (float) b2_temp;
  b_coeff[3] = (float) b3_temp;
/*
for (int i = 0; i <= 3; i++)
  {
    Serial.println(a_coeff[i],10);
  }
  Serial.println();
  for (int i = 0; i <= 3; i++)
  {
    Serial.println(b_coeff[i],10);
  }
*/
}

void audioPassThrough(){
  leftOut[outputIndex] = leftIn[curSample];
  rightOut[outputIndex] = rightIn[curSample];    
}

void intvlFunction(void){
  
  if (processCount++ == 500)
  {
    processCount = 0;
    updateLCD = true;
  }

  while(Q_in_L.available() && Q_in_R.available())
    audioProcess();
}

void buttonFlag(){
  encBtn.update();
  btn.update();
  if(encBtn.risingEdge() || btn.risingEdge()){ 
    btnFlag = true;
  }
}

void displayMenu()
{   
    lcd.print(currentMenu->title);
    for(int i = 0; i < currentMenu->numItems; i++){
      lcd.setCursor(0,i+1);
      if(i > 2){
          lcd.setCursor(10,count+1);
          count++;
      }
      if(currentMenu->selected == i){
          lcd.print(">");
      }
      else{
          lcd.print(" ");
      }
      lcd.print(currentMenu->items[i].title);
      lcd.setCursor(10,0);

      if(currentMenu->items[i].param != NULL && i != 0){
        lcd.setCursor(13,0);
        int* varList = (int*) currentMenu->items[i].param;
        int* val = (int*) varList[1];
        lcd.print(*val);
      }
      lcd.setCursor(0,0);
    }
    count = 0;
}

void setup(void){
  lcd.init();
  lcd.backlight();
  displayMenu();
  // Audio connections require memory. and the record queue
  // uses this memory to buffer incoming audio
  // The correct amount of memory usage is found using AudioMemoryUsageMax()
  AudioMemory(138);
  pinMode(ENC_BUTTON, INPUT_PULLDOWN);
  pinMode(BUTTON, INPUT_PULLDOWN);
  pinMode(LED, OUTPUT);
  attachInterrupt(digitalPinToInterrupt(ENC_BUTTON), buttonFlag, RISING );
  attachInterrupt(digitalPinToInterrupt(BUTTON), buttonFlag, RISING );
  // Start the record queues
  Q_in_L.begin();
  Q_in_R.begin();
  //Run audio process every 1ms and set to 
  audioProcesser.begin(intvlFunction, 1000);
  aic23.enable();
  //must set both for now
  aic23.inputLevel(0.65);
  curPreampGain = (int)(32 * 0.65);
}

void loop(void){ 

  int8_t newPos = (enc1.read()>>2);
  //Extract sign. If encoder not moved, sign = 0
  int8_t sign = (newPos > 0) - (newPos < 0);
  int8_t input = 0;
  input = sign * 1;
  
  if(btnFlag){
    input = 2;
    btnFlag = false;
  } 

  switch(input)
  {
    //down
    case 1:
        currentMenu->selected++;
        if(currentMenu->selected >= currentMenu->numItems){
            currentMenu->selected = currentMenu->numItems - 1;    
        }
        enc1.write(0);
        break;
    //up
    case -1:
        if(currentMenu->selected > 0){
            //hack to fix PEC11R encoder counting by 2 using encoder library
            if(toggle==2){
              currentMenu->selected--;
              toggle = 0;
            }
            toggle++;
        }
        enc1.write(0);
        break;
    //execute
    case 2:
        lcd.clear();
        //Check for submenu
        if(currentMenu->items[currentMenu->selected].subMenu != NULL){
            currentMenu = currentMenu->items[currentMenu->selected].subMenu;
        }
        //Check if it has an action(function ptr)
        else if (currentMenu->items[currentMenu->selected].action != NULL){
          int* varList = (int*) currentMenu->items[currentMenu->selected].param;
          int* val = (int*) varList[0];
          //Hack to invoke volume update
          if(*val == preamp){
                aic23.inputLevel((float)curPreampGain/32.0);
          }
            currentAction = currentMenu->items[currentMenu->selected].action;
            currentParam = currentMenu->items[currentMenu->selected].param;
        }
        // Go back
        else if (currentMenu->parentMenu != NULL){
            currentMenu = currentMenu->parentMenu;
        }
        // Jump back to main menu if nothing else
        else{
            currentMenu = &mainMenu;
        }
        break;
      default:
        break;
    }
  //execute function
  if (currentAction != NULL){
      //reset if function is running function
      if (currentAction(currentParam) == true)
      {
          currentAction = NULL;
          currentParam = NULL;
      }
  }
  //skip if no user input. Currently screen only changes with user input
  if(input != 0) 
  {
    updateLCD=false;
    displayMenu();
  }
}


