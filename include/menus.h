#ifndef MENUS_H
#define MENUS_H

#include <stdio.h>

#define NUMELEMENTS(X) (sizeof(X) / sizeof(X[0]))
#define LED 15

struct MENUITEM
{
  char *title;                // item title
  bool (*action)(void *ptr);  // function to execute
  void *param;                // parameter for function to execute
  struct MENU *subMenu;       // submenu for this menu item
};

struct MENU
{
  char *title;              // menu title
  MENUITEM *items;          // menu items
  int numItems;             // number of menu items
  int selected;             // item that is selected
  struct MENU *parentMenu;  // parent menu of this menu
};

enum effects{
  passThrough=0,
  movingAverage,
  echo,
  distortion,
  fuzz,
  flanger,
  equalizer,
  octaver,
  tuner,
  tremelo,
  binarize,
  preamp
};

int pt = passThrough;
int* ptVar = &pt;
int* curEffect = ptVar;
//Current value, min/max bounds, and increment value for each menu item 
int distortionPedal = distortion, curGain=5, curBoost=1, gainIncVal= 1,boostIncVal= 1,
                      gainLower = 1, gainUpper = 25, boostLower = 1, boostUpper = 5,
                      curAsym=0, asymIncVal=1, asymLower=0, asymUpper=2;
void* gainVars[5] = {&distortionPedal, &curGain,&gainIncVal, &gainLower, &gainUpper};
void* boostVars[5] = {&distortionPedal, &curBoost,&boostIncVal, &boostLower, &boostUpper};
void* asymVars[5] = {&distortionPedal, &curAsym, &asymIncVal, &asymLower, &asymUpper};
int echoPedal = echo, curDelay=5000, curFeedback=50, delayIncVal= 250, feedbackIncVal= 5,
                feedbackLower = 10, feedbackUpper = 100, delayLower = 0, delayUpper = 16000;
void* delayVars[5] = {&echoPedal, &curDelay,&delayIncVal, &delayLower, &delayUpper};
void* feedbackVars[5] = {&echoPedal, &curFeedback, &feedbackIncVal, &feedbackLower, &feedbackUpper};
int fuzzPedal = fuzz, curLimit=4000, curFiltLen=25, curFuzzGain=1,limitIncVal=250, filtLenIncVal=5,
                fuzzGainIncVal=1, limitUpper = 10000, limitLower = 2000, filtLenUpper = 75, 
                filtLenLower = 1,fuzzGainUpper = 5, fuzzGainLower=1;
void* limitVars[5] = {&fuzzPedal, &curLimit, &limitIncVal, &limitLower, &limitUpper};
void* filtLenVars[5] = {&fuzzPedal, &curFiltLen, &filtLenIncVal, &filtLenLower, &filtLenUpper};
void* fuzzGainVars[5] = {&fuzzPedal,&curFuzzGain, &fuzzGainIncVal, &fuzzGainLower,&fuzzGainUpper};
int flangerPedal = flanger, curSpeed = 1, curDepth = 40, speedIncVal = 1, depthIncVal =2,
                    speedLower = 0, speedUpper = 11, depthLower = 0, depthUpper = 88;
void* speedVars[5] = {&flangerPedal, &curSpeed, &speedIncVal, &speedLower, &speedUpper};
void* depthVars[5] = {&flangerPedal, &curDepth, &depthIncVal, &depthLower, &depthUpper};
int equalizerPedal = equalizer, curLow = 50, curMid = 50, curHigh = 50, eqIncVal = 2, 
                    eqUpper = 100, eqLower = 0, curEqGain =2, eqGainIncVal =1, eqGainUpper = 5,
                    eqGainLower = 1;
void* eqLowVars[5] = {&equalizerPedal, &curLow, &eqIncVal, &eqLower, &eqUpper};
void* eqMidVars[5] = {&equalizerPedal, &curMid, &eqIncVal, &eqLower, &eqUpper};
void* eqHighVars[5] = {&equalizerPedal, &curHigh, &eqIncVal, &eqLower, &eqUpper};
void* eqGainVars[5] = {&equalizerPedal, &curEqGain, &eqGainIncVal, &eqGainLower, &eqGainUpper};
int tremeloPedal = tremelo, curTremSpeed = 4, curTremDepth = 50, curTremMix = 90, tremSpeedInc = 1,
                    tremDepthInc = 2, tremMixInc =1, tremSpeedUpper=15, tremSpeedLower=1, tremDepthUpper=100,
                    tremDepthLower=0;
void* tremSpeedVars[5] = {&tremeloPedal, &curTremSpeed, &tremSpeedInc, &tremSpeedLower, &tremSpeedUpper};
void* tremDepthVars[5] = {&tremeloPedal, &curTremDepth, &tremDepthInc, &tremDepthLower, &tremDepthUpper};
int preampPedal = preamp, curPreampGain = 23, preampIncVal = 1, preampLower =0, preampUpper = 31; //0b10111 Default aci23 value for 0dB gain
void* preampVars[5] = {&preampPedal, &curPreampGain, &preampIncVal,&preampLower, &preampUpper};

bool effectToggle(void* param){
  int* varList = (int*)param;
  //Toggle back to pass through for same effect
  if(*(int*)varList[0] == *curEffect){
      curEffect = ptVar;
      digitalWrite(LED, LOW);
  }
  else{
    digitalWrite(LED, HIGH);
    curEffect = (int*) varList[0];
  }
  return true;
} 

bool increment(void *param){
    int* varList = (int*)param;
    int* var = (int*) varList[1];
    int* incVal = (int*) varList[2];
    int* upperBound = (int*) varList[4];
    int temp = *var + *incVal;
    if(temp > *upperBound)
        return true;
    else *var += *incVal;
    return true;
}

bool decrement(void *param){
    int* varList = (int*)param;
    int* var = (int*) varList[1];
    int* incVal = (int*) varList[2];
    int* lowerBound = (int*)varList[3];
    int temp = *var - *incVal;
    if(temp < *lowerBound)
        return true;
    else *var -= *incVal;
    return true;
}

extern MENU mainMenu;
extern MENU effectsMenu;
extern MENU preampMenu;

extern MENU distortionMenu;
extern MENU boostMenu;
extern MENU gainMenu;
extern MENU asymMenu;

extern MENU echoMenu;
extern MENU delayMenu;
extern MENU feedbackMenu;

extern MENU tunerMenu;

extern MENU fuzzMenu;
extern MENU limitMenu;
extern MENU filtLenMenu;
extern MENU fuzzGainMenu;

extern MENU flangerMenu;
extern MENU speedMenu;
extern MENU depthMenu;

extern MENU equalizerMenu;
extern MENU midMenu;
extern MENU lowMenu;
extern MENU highMenu;

extern MENU tremeloMenu;
extern MENU tremDepthMenu;
extern MENU tremSpeedMenu;

/* Begin Distortion Menu*/

MENUITEM asymMenuItems[] =
{
    {(char*)"+",increment,asymVars,NULL},
    {(char*)"-",decrement,asymVars,NULL},
    {(char*)"Back",NULL,NULL,NULL}
};

MENU asymMenu =
{
    (char*)"Asymetrical", asymMenuItems,NUMELEMENTS(asymMenuItems),0,&distortionMenu
};

MENUITEM gainMenuItems[] =
{
    {(char*)"+",increment,gainVars, NULL},
    {(char*)"-",decrement,gainVars, NULL},
    {(char*)"Back",NULL,NULL,NULL}
};

MENU gainMenu =
{
    (char*)"Gain",gainMenuItems,NUMELEMENTS(gainMenuItems),0,&distortionMenu
};

MENUITEM boostMenuItems[] =
{
    {(char*)"+",increment,boostVars,NULL},
    {(char*)"-",decrement,boostVars,NULL},
    {(char*)"Back",NULL,NULL,NULL}
};

MENU boostMenu =
{
    (char*)"Boost",boostMenuItems,NUMELEMENTS(boostMenuItems),0,&distortionMenu
};

MENUITEM distortionMenuItems[] =
{
    {(char*)"Toggle", effectToggle,gainVars, NULL},
    {(char*)"Gain",NULL,NULL,&gainMenu},
    {(char*)"Boost",NULL,NULL,&boostMenu},
    {(char*)"Asymetric",NULL,NULL,&asymMenu},
    {(char*)"Back",NULL,NULL,NULL}
};

MENU distortionMenu =
{
    (char*)"Distortion",distortionMenuItems,NUMELEMENTS(distortionMenuItems),0,&effectsMenu
};
/* End Distortion Menu*/

/* Begin Echo Menu*/
MENUITEM delayMenuItems[] =
{
    {(char*)"+",increment,delayVars, NULL},
    {(char*)"-",decrement,delayVars, NULL},
    {(char*)"Back",NULL,NULL,NULL}
};

MENU delayMenu =
{
    (char*)"Delay",delayMenuItems,NUMELEMENTS(delayMenuItems),0,&echoMenu
};

MENUITEM feedbackMenuItems[] =
{
    {(char*)"+",increment,feedbackVars,NULL},
    {(char*)"-",decrement,feedbackVars,NULL},
    {(char*)"Back",NULL,NULL,NULL}
};

MENU feedbackMenu =
{
    (char*)"Feedback",feedbackMenuItems,NUMELEMENTS(feedbackMenuItems),0,&echoMenu
};

MENUITEM echoMenuItems[] =
{
    {(char*)"Toggle", effectToggle,delayVars, NULL},
    {(char*)"Delay",NULL,NULL,&delayMenu},
    {(char*)"Feedback",NULL,NULL,&feedbackMenu},
    {(char*)"Back",NULL,NULL,NULL}
};

MENU echoMenu =
{
    (char*)"Echo", echoMenuItems, NUMELEMENTS(echoMenuItems),0,&effectsMenu
};
/* End Echo Menu*/

/*Begin Fuzz Menu*/
MENUITEM fuzzGainMenuItems[] =
{
    {(char*)"+",increment,fuzzGainVars, NULL},
    {(char*)"-",decrement,fuzzGainVars, NULL},
    {(char*)"Back",NULL,NULL,NULL}
};

MENU fuzzGainMenu =
{
    (char*)"Gain",fuzzGainMenuItems,NUMELEMENTS(fuzzGainMenuItems),0,&fuzzMenu
};

MENUITEM filtLenMenuItems[] =
{
    {(char*)"+",increment,filtLenVars, NULL},
    {(char*)"-",decrement,filtLenVars, NULL},
    {(char*)"Back",NULL,NULL,NULL}
};

MENU filtLenMenu =
{
    (char*)"Lowpass",filtLenMenuItems,NUMELEMENTS(filtLenMenuItems),0,&fuzzMenu
};

MENUITEM limitMenuItems[] =
{
    {(char*)"+",increment,limitVars,NULL},
    {(char*)"-",decrement,limitVars,NULL},
    {(char*)"Back",NULL,NULL,NULL}
};

MENU limitMenu =
{
    (char*)"Limit",limitMenuItems,NUMELEMENTS(limitMenuItems),0,&fuzzMenu
};

MENUITEM fuzzMenuItems[] =
{
    {(char*)"Toggle", effectToggle, limitVars, NULL},
    {(char*)"Limit",NULL,NULL,&limitMenu},
    {(char*)"Lowpass",NULL,NULL,&filtLenMenu},
    {(char*)"Gain",NULL,NULL,&fuzzGainMenu},
    {(char*)"Back",NULL,NULL,NULL}
};

MENU fuzzMenu =
{
    (char*)"Fuzz405", fuzzMenuItems, NUMELEMENTS(fuzzMenuItems),0,&effectsMenu
};
/*End Fuzz Menu*/

/*Begin Flanger Menu*/
MENUITEM speedMenuItems[] =
{
    {(char*)"+",increment,speedVars,NULL},
    {(char*)"-",decrement,speedVars,NULL},
    {(char*)"Back",NULL,NULL,NULL}
};

MENU speedMenu =
{
    (char*)"Speed",speedMenuItems,NUMELEMENTS(speedMenuItems),0,&flangerMenu
};

MENUITEM depthMenuItems[] =
{
    {(char*)"+",increment,depthVars,NULL},
    {(char*)"-",decrement,depthVars,NULL},
    {(char*)"Back",NULL,NULL,NULL}
};

MENU depthMenu =
{
    (char*)"Depth",depthMenuItems,NUMELEMENTS(depthMenuItems),0,&flangerMenu
};

MENUITEM flangerMenuItems[] =
{
    {(char*)"Toggle", effectToggle, speedVars, NULL},
    {(char*)"Speed",NULL,NULL, &speedMenu},
    {(char*)"Depth",NULL,NULL, &depthMenu},
    {(char*)"Back",NULL,NULL,NULL}
};

MENU flangerMenu = {
    (char*)"Flanger", flangerMenuItems, NUMELEMENTS(flangerMenuItems),0,&effectsMenu
};
/*End Flanger Menu*/

/*Begin Tremelo Menu*/

MENUITEM tremSpeedMenuItems[] =
{
    {(char*)"+",increment,tremSpeedVars,NULL},
    {(char*)"-",decrement,tremSpeedVars,NULL},
    {(char*)"Back",NULL,NULL,NULL}
};

MENU tremSpeedMenu =
{
    (char*)"Speed",tremSpeedMenuItems,NUMELEMENTS(tremSpeedMenuItems),0,&tremeloMenu
};

MENUITEM tremDepthMenuItems[] =
{
    {(char*)"+",increment,tremDepthVars,NULL},
    {(char*)"-",decrement,tremDepthVars,NULL},
    {(char*)"Back",NULL,NULL,NULL}
};

MENU tremDepthMenu =
{
    (char*)"Depth",tremDepthMenuItems,NUMELEMENTS(tremDepthMenuItems),0,&tremeloMenu
};

MENUITEM tremeloMenuItems[] = {
    {(char*)"Toggle", effectToggle, tremSpeedVars, NULL},
    {(char*)"Speed",NULL,NULL, &tremSpeedMenu},
    {(char*)"Depth",NULL,NULL, &tremDepthMenu},
    {(char*)"Back",NULL,NULL,NULL}
};

MENU tremeloMenu = 
{
    (char*)"Tremelo", tremeloMenuItems, NUMELEMENTS(tremeloMenuItems),0,&effectsMenu
};
/*End Tremelo Menu*/

MENUITEM effectsMenuItems[] =
{
    {(char*)"Distort",NULL,NULL,&distortionMenu},
    {(char*)"Fuzz405",NULL,NULL,&fuzzMenu},
    {(char*)"Echo",NULL,NULL,&echoMenu},
    {(char*)"Flanger", NULL, NULL, &flangerMenu},
    {(char*)"Tremelo", NULL,NULL, &tremeloMenu},
    {(char*)"Back",NULL,NULL,NULL}
};

MENU effectsMenu =
{
    (char*)"Effects",effectsMenuItems, NUMELEMENTS(effectsMenuItems),0,&mainMenu
};

/*Begin Equalizer Menu*/

MENUITEM highMenuItems[] = {
    {(char*)"+",increment,eqHighVars,NULL},
    {(char*)"-",decrement,eqHighVars,NULL},
    {(char*)"Back",NULL,NULL,NULL}
};

MENU highMenu{
    (char*)"High",highMenuItems,NUMELEMENTS(highMenuItems),0,&equalizerMenu
};

MENUITEM midMenuItems[] = {
    {(char*)"+",increment,eqMidVars,NULL},
    {(char*)"-",decrement,eqMidVars,NULL},
    {(char*)"Back",NULL,NULL,NULL}
};

MENU midMenu{
    (char*)"Mid",midMenuItems,NUMELEMENTS(midMenuItems),0,&equalizerMenu
};

MENUITEM lowMenuItems[] = {
    {(char*)"+",increment,eqLowVars,NULL},
    {(char*)"-",decrement,eqLowVars,NULL},
    {(char*)"Back",NULL,NULL,NULL}
};

MENU lowMenu{
    (char*)"Bass",lowMenuItems,NUMELEMENTS(lowMenuItems),0,&equalizerMenu
};

MENUITEM eqGainMenuItems[] = {
    {(char*)"+",increment,eqGainVars,NULL},
    {(char*)"-",decrement,eqGainVars,NULL},
    {(char*)"Back", NULL,NULL,NULL}
};

MENU eqGainMenu{
    (char*)"Gain",eqGainMenuItems, NUMELEMENTS(eqGainMenuItems),0,&equalizerMenu
};

MENUITEM equalizerMenuItems[] =
{
    {(char*)"Toggle", effectToggle, eqLowVars, NULL},
    {(char*)"Bass",NULL,NULL,&lowMenu},
    {(char*)"Mid",NULL,NULL,&midMenu},
    {(char*)"Treble",NULL,NULL,&highMenu},
    {(char*)"Gain", NULL,NULL,&eqGainMenu},
    {(char*)"Back",NULL,NULL,NULL}
};

MENU equalizerMenu =
{
    (char*)"Bassman",equalizerMenuItems, NUMELEMENTS(equalizerMenuItems),0,&mainMenu
};
/*End Equalizer Menu*/

MENUITEM preampMenuItems[] = 
{
    {(char*)"+",increment,preampVars,NULL},
    {(char*)"-",decrement,preampVars,NULL},
    {(char*)"Back", NULL,NULL,NULL}
};

MENU preampMenu = 
{
    (char*)"Preamp", preampMenuItems, NUMELEMENTS(preampMenuItems),0,&mainMenu
};
MENUITEM mainMenuItems[] =
{
    {(char*)"Effects",NULL,NULL,&effectsMenu},
    {(char*)"Tone Stack",NULL,NULL,&equalizerMenu},
    {(char*)"Preamp",NULL,NULL, &preampMenu}
};

MENU mainMenu =
{
    (char*)"Main menu",mainMenuItems,NUMELEMENTS(mainMenuItems),0,NULL
};



#endif