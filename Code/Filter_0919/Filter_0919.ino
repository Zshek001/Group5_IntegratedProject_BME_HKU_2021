#include "DirectFormII.h"


//constant declaration//
const int loop_frequency = 80;                     //loop frequency in Hz//
const int loop_period = 1000000 / loop_frequency;  //loop period in us//
const int PDdelay = loop_period / 2;                    //delay time for PD response in us//
const int erase_period = 5000000;          //Clear the buffer and calculate for every 5 seconds//


const int analog_MAX = 4095;                //Maximum of the analog reading/writing//
const double Vmax = 3.3;                    //Maximum of the output voltage//

// Pin declaration //
const int RED = DAC0;  //Red LED//
const int NIR = DAC1;  //NIR LED//
const int PD = A0;  //photodetector//



//Variable declaration//
int i = 0;           //loop times//
int loop_start = 0;  //starting time of this loop//
int loop_end = 0;    //ending time of this loop//
int detected_RED = 0;//reading of PD for RED signal//
int detected_NIR = 0;//reading of PD for NIR signal//
int filtered_RED = 0;//filtered data of RED signal//
int filtered_NIR = 0;//filtered data of NIR signal//
double Vout = 3.3;//Output woltage of LED light//
//int LED[2] = {0,0};



//Declare of objects//
const int N_LEDarray = 11;                      //Length of the buffer signal array(must be odd)//
const int heartrate_constrain[2] = {30,150};
class Signal
{
  //temporary data of extrme point//
  public: float maximum;
  public: float minimum;
  
  public: float heartrate;

  //Declare the array for store the detected LED signal and its curve//

  float LEDarray[N_LEDarray] = {};
  int LEDcur = 0;
  bool label = true;    //True for exceeding threshold//


//analyze the signal//
  public: int signal_analyze(float current){

    current = DirectFormIISOS(current, Coeffs_sample01, p_bx_Gain_sample01); //filter//
    putdata(current);  //put the filtered data into the buffer//
    int state = find_peak(LEDcur); //determine whether this cursor points at a peak or trough//
    if(state == 1 && current > (maximum + minimum) / 2){ // if there is peak, calculate the period and the maximum//

      maximum = adapt_max(current);
    }
    else if(state = -1 && current < (maximum + minimum) / 2){//calculate minimum at trough//  
      heartrate = find_period(LEDcur);
      minimum = adapt_min(current);
    }


    return 0;
  }

  //find peak, return 1 when finding peak, return -1 when finding trough//
  int find_peak(int cur){
    if(tem_cur < N_LEDarray) return 0; //return false if the buffer has not been filled up//

    float middle_data = LEDarray[(cur - N_LEDarray / 2) % N_LEDarray]; //find the middle data in the buffer//
    int sign = 0;
    
    for(int j = 0; j < N_LEDarray; j++){
      float current_data = LEDarray[(cur - j) % N_LEDarray];
      if(middle_data == current_data) continue; //move to the next number when equal//
      
      if(sign = 0)
        sign = (middle_data > current_data)? 1 : -1; //set the sign//
      
      if((sign == 1 && middle_data < current_data) || (sign == -1 && middle_data > current_data))
        return 0;
      
    }
    return sign; 
  }

  //using adaptive threshold to calculate the maximum and minimum//
  float max_threshold[2] = {0,0};
  
  float adapt_max(float num){ 
    if(max_threshold[0] == 0 && max_threshold[1] == 0){
      max_threshold[0] == 0.8 * num;
      max_threshold[1] == 1.2 * num;
      return -1;
    }
    if(num >= max_threshold[0] && num <= max_threshold[1]){
      max_threshold[0] = (max_threshold[0] + num * 1.1) / 2;
      max_threshold[1] = (max_threshold[1] + num * 0.9) / 2;
    }
    else if(num < max_threshold[0]){
      max_threshold[0] = (max_threshold[0] + num * 0.9) / 2;
      max_threshold[1] = (max_threshold[1] + num * 0.9) / 2;
    }
    else if(num > max_threshold[1]){
      max_threshold[0] = (max_threshold[0] + num * 1.1) / 2;
      max_threshold[1] = (max_threshold[1] + num * 1.1) / 2;
    }
    return (max_threshold[0] + max_threshold[1]) / 2;
  }

  float min_threshold[2] = {0,0};
  
  float adapt_min(float num){ 
    if(min_threshold[0] == 0 && min_threshold[1] == 0){
      min_threshold[0] == 0.8 * num;
      min_threshold[1] == 1.2 * num;
      return -1;
    }
    if(num >= min_threshold[0] && num <= min_threshold[1]){
      min_threshold[0] = (min_threshold[0] + num * 1.1) / 2;
      min_threshold[1] = (min_threshold[1] + num * 0.9) / 2;
    }
    else if(num < min_threshold[0]){
      min_threshold[0] = (min_threshold[0] + num * 0.9) / 2;
      min_threshold[1] = (min_threshold[1] + num * 0.9) / 2;
    }
    else if(num > min_threshold[1]){
      min_threshold[0] = (min_threshold[0] + num * 1.1) / 2;
      min_threshold[1] = (min_threshold[1] + num * 1.1) / 2;
    }
    return (min_threshold[0] + min_threshold[1]) / 2;
  }

  //find the period//
  int tem_cur = 0;           //store the last cursor exceeding the threshold//
  int heartrate_threshold[2] = {60,120};

  int find_period(int cur){
    if(tem_cur == 0){
      tem_cur = cur;
      return 0;
    }
    int heartbeat = 60* loop_frequency / 4 / (cur - tem_cur) ;
    if(heartbeat < heartrate_constrain[0] || heartbeat > heartrate_constrain[1]) return -1;

    if(heartbeat >= heartrate_threshold[0] && heartrate <= heartrate_threshold[1]){
      heartrate_threshold[0] = (heartrate_threshold[0] + heartbeat * 1.1) / 2;
      heartrate_threshold[1] = (heartrate_threshold[1] + heartbeat * 0.9) / 2;
    }
    else if(heartbeat < heartrate_threshold[0]){
      heartrate_threshold[0] = (heartrate_threshold[0] + heartbeat * 0.9) / 2;
      heartrate_threshold[1] = (heartrate_threshold[1] + heartbeat * 0.9) / 2;
    }
    else if(heartbeat > heartrate_threshold[1]){
      heartrate_threshold[0] = (heartrate_threshold[0] + heartbeat * 1.1) / 2;
      heartrate_threshold[1] = (heartrate_threshold[1] + heartbeat * 1.1) / 2;
    }
    
    tem_cur = cur;
    return (heartrate_threshold[0] + heartrate_threshold[1]) / 2;
    
  }

//clear the buffer//
  public: void cleardata(){
    maximum = 0;
    minimum = analog_MAX;
    heartrate = 0;
    tem_cur = 0; 
    LEDcur = 0;
    heartrate_threshold[0] = 60;
    heartrate_threshold[1] = 120;
    max_threshold[0] = 0;
    max_threshold[1] = 0;
    min_threshold[0] = 0;
    min_threshold[1] = 0;
  }

//return the cursor//
  public: int getcur(){
    return LEDcur % N_LEDarray;
  }

//input the data to the array//
  public: void putdata(double current){
    LEDcur++;
    LEDarray[getcur()] = current;
  }

//return the currentdata in the array//
  public: double get_currentdata(){
    return LEDarray[getcur() % N_LEDarray];
  }


};

Signal RED_Signal, NIR_Signal;


       

void setup() {
  // put your setup code here, to run once:
  Serial.begin(38400);
  //Setup the pinmode//
  pinMode(RED,OUTPUT);
  pinMode(NIR,OUTPUT);
  pinMode(PD,INPUT);

  // Set the resolution(0 - 4095)//
  analogReadResolution(12);
  analogWriteResolution(12);

  //initiallize//
  RED_Signal.cleardata();
  NIR_Signal.cleardata();

}

void loop() {
  // put your main code here, to run repeatedly:
    
  loop_start = micros();  //Record the start time of the loop//

  //generate PD signal//
  switch (i % 4){
    case 0:
    analogWrite(NIR,(Vout) * analog_MAX / Vmax);
    //LED[0] = 1;
    break;

    case 1:
    analogWrite(NIR,0);
    //LED[0] = 0;
    break;

    case 2:
    analogWrite(RED, Vout * analog_MAX / Vmax);
    //LED[1] = 10;
    break;

    case 3:
    analogWrite(RED,0);
    //LED[1] = 0;
    break;

    default:
    break;
  }

  //detect PD signal//
   delayMicroseconds(loop_start + PDdelay - micros());
    switch (i % 4){
      case 0:
      detected_NIR = analogRead(PD);
      NIR_Signal.putdata(detected_NIR); //store the data in the array//

      //detected_NIR = LED[0] + LED[1];
      break;

      case 2:
      detected_RED = analogRead(PD);
      RED_Signal.putdata(detected_RED); //store the data in the array//
      //if (detected_RED > (analog_MAX * 0.9)){
      //  Vout -= 0.1;
    //  }
      //detected_RED = LED[1] + LED[0];
      break;
      
      default:
      break;
    }
    
//Test for outputing//
  RED_Signal.signal_analyze(RED_Signal.get_currentdata());
  Serial.print(RED_Signal.get_currentdata());
  Serial.print(' ');
  //Serial.print(RED_Signal.maximum);
  //Serial.print(' ');
 // Serial.print(RED_Signal.minimum);
  Serial.print(' ');
  Serial.println(NIR_Signal.get_currentdata());

  i++;

  //delay to fix the loop time//
  //if(i >= erase_period / loop_period){
  //  RED_Signal.cleardata();
 //   NIR_Signal.cleardata();
  //  i = 0;

 // }
   if(i > 1000)
    while(1);
  loop_end = micros();
  if(loop_start + loop_period - loop_end > 0) delayMicroseconds(loop_start + loop_period - loop_end);
  else Serial.println("Error");
}
