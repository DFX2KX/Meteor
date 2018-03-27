#include <Gamebuino-Meta.h>
const uint8_t myshipData[] = {8, 8, 8, 0, 0, 0xFF, 1, 0x00, 0x00,0x00, 0x00,0x00, 0x07,0x70, 0x00,0x00, 0x07,0x70, 0x00,0x00, 0x70,0x07, 0x00,0x00, 0x70,0x07, 0x00,0x07, 0x00,0x00, 0x70,0x07, 0x00,0x00, 0x70,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x07, 0x70,0x00, 0x77,0x77, 0x70,0x77, 0x00,0x07, 0x00,0x00, 0x00,0x07, 0x00,0x00, 0x00,0x70, 0x00,0x00, 0x00,0x70, 0x00,0x00, 0x00,0x00, 0x00,0x07, 0x70,0x00, 0x00,0x00, 0x07,0x70, 0x00,0x00, 0x00,0x07, 0x70,0x00, 0x00,0x07, 0x70,0x00, 0x07,0x70, 0x00,0x07, 0x70,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x70, 0x00,0x00, 0x00,0x77, 0x00,0x00, 0x00,0x07, 0x00,0x00, 0x00,0x07, 0x00,0x77, 0x00,0x00, 0x70,0x07, 0x77,0x07, 0x70,0x00, 0x00,0x77, 0x70,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x07, 0x00,0x00, 0x70,0x07, 0x00,0x00, 0x70,0x00, 0x70,0x07, 0x00,0x00, 0x70,0x07, 0x00,0x00, 0x07,0x70, 0x00,0x00, 0x07,0x70, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x70, 0x00,0x00, 0x07,0x70, 0x00,0x00, 0x07,0x00, 0x00,0x00, 0x07,0x00, 0x00,0x00, 0x70,0x00, 0x77,0x00, 0x77,0x07, 0x70,0x00, 0x77,0x70, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x07, 0x70,0x00, 0x07,0x70, 0x00,0x07, 0x70,0x00, 0x00,0x07, 0x70,0x00, 0x00,0x00, 0x07,0x70, 0x00,0x00, 0x00,0x07, 0x70,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x00,0x00, 0x77,0x77, 0x00,0x00, 0x77,0x00, 0x77,0x00, 0x70,0x00, 0x00,0x00, 0x70,0x00, 0x00,0x00, 0x07,0x00, 0x00,0x00, 0x07,0x00, 0x00};
Image myShipImage = Image(myshipData);
// direction vector class
class dirVector {
   public:
   int dir, mag;
   void setVal(int, int);
   float getXpart();
   float getYpart();
   dirVector add(dirVector);
   dirVector sub(dirVector);
 };
 
void dirVector::setVal (int dirnew, int magnew){
    mag = magnew;
    dir = dirnew;
};

float dirVector::getXpart(){
  float rads = ((dir % 359) * 71) / 4068;
    return (mag * cos(rads));
};

float dirVector::getYpart(){
  float rads = ((dir % 359) * 71) / 4068;
    return (mag * sin(rads));
};

dirVector dirVector::add(dirVector vecB) {
float deg, len, sumx, sumy;
  sumx = getXpart() + vecB.getXpart();
  sumy = getYpart() + vecB.getYpart();
  deg = atan(sumy/sumx);
  deg = ((deg) * 57296 / 1000);
  if (sumx > 0 && sumy > 0){
  deg = deg;
  };
  if (sumx < 0 && sumy < 0){
  deg = deg + 180 % 359;
  };
  if (sumx < 0 && sumy > 0){
  deg = deg + 180 % 359;
  };
  if (sumx > 0 && sumy > 0){
  deg = deg + 360 % 359;
  };
  len = sqrt( sq(sumx) + sq(sumy));
  dirVector output;
  output.setVal(deg,len);
  return output;
};

dirVector dirVector::sub(dirVector vecB) {
float deg, len, sumx, sumy;
  sumx = getXpart() - vecB.getXpart();
  sumy = getYpart() - vecB.getYpart();
  deg = atan(sumy/sumx);
  deg = ((deg) * 57296 / 1000);
    if (sumx > 0 && sumy > 0){
  deg = deg;
  };
  if (sumx < 0 && sumy < 0){
  deg = deg + 180 % 359;
  };
  if (sumx < 0 && sumy > 0){
  deg = deg + 180 % 359;
  };
  if (sumx > 0 && sumy > 0){
  deg = deg + 360 % 359;
  };
  len = sqrt( sq(sumx) + sq(sumy));
  dirVector output;
  output.setVal(deg,len);
  return output;
};
class laser{
  public:
  int x,y,heading;
  dirVector mv;
  
  };
class ship {
  public:
  int x, y, heading;
  dirVector mv;
  void applyThrust (int);
  void applyDrag(int);
  void setHeading(int);
  void draw();
};

void ship::setHeading (int hdg){
  heading = hdg;
};

void ship::applyThrust (int mag){
  dirVector thrustvec;
  if (mag > 0){
    thrustvec.setVal(heading - 90 % 359, mag);
  } else{
    thrustvec.setVal((heading - 90) + 180 % 359, abs(mag));
  };
  dirVector newvec = mv.add(thrustvec);
  mv.setVal(newvec.dir,newvec.mag);
};

void ship::applyDrag (int mag) {
  dirVector dragvec;
  dragvec.setVal((mv.dir - 180) % 359, mag);
  dirVector newvec = mv.add(dragvec);
  mv.setVal(newvec.dir,newvec.mag);
};

void ship::draw(){
  int frame;
switch (heading){
  case 0:
    frame =0;
    break;
  case 45:
    frame =1;
    break;
  case 90:
    frame =2;
    break;
  case 135:
    frame =3;
    break;
  case 180:
    frame =4;
    break;
  case 225:
    frame =5;
    break;
  case 270:
    frame =6;
    break;
  case 315:
    frame =7;
    break;
};
 // gb.display.drawImage(x + 4, y + 4, myShipImage, 0, 0 + (8 * frame),8, 8);
 myShipImage.setFrame(frame);
 gb.display.drawImage(x+4,y+4, myShipImage);
};

ship myShip;
void setup() {
  // put your setup code here, to run once:
  gb.begin();
myShip.x = gb.display.width()/2;
myShip.y = gb.display.height()/2;
myShip.heading=0;
gb.display.setTransparentColor(BLACK);
}

void loop() {
  // put your main code here, to run repeatedly:
  while (!gb.update());
  myShip.x += myShip.mv.getXpart();
  myShip.y += myShip.mv.getYpart();

  if (myShip.x < 0) {
    myShip.x = gb.display.width();  
  };
  if (myShip.x > gb.display.width()) {
    myShip.x = 0;  
  };
  if (myShip.y < 0) {
    myShip.y = gb.display.height();  
  };
  if (myShip.y > gb.display.height()) {
    myShip.y = 0;  
  };
  gb.display.clear();
  if (myShip.mv.mag >30) {
    myShip.mv.mag =30;
  };
  
  if (myShip.mv.mag < -30) {
    myShip.mv.mag = -30;
  };
  
  if (gb.buttons.pressed(BUTTON_UP)){
    myShip.applyThrust(1);
  };

  if (gb.buttons.pressed(BUTTON_DOWN)){
    myShip.applyThrust(-1);
  };
  
  if (gb.buttons.pressed(BUTTON_LEFT)){
    switch (myShip.heading){
      case 0:
      myShip.heading = 315;
      break;
      case 315:
      myShip.heading = 270;
      break;
      case 270:
      myShip.heading = 225;
      break;
      case 225:
      myShip.heading = 180;
      break;
      case 180:
      myShip.heading = 135;
      break;
      case 135:
      myShip.heading = 90;
      break;
      case 90:
      myShip.heading = 45;
      break;
      case 45:
      myShip.heading = 0;
      break;
    };
  };

  if (gb.buttons.pressed(BUTTON_RIGHT)){
    switch (myShip.heading){
      case 0:
      myShip.heading = 45;
      break;
      case 45:
      myShip.heading = 90;
      break;
      case 90:
      myShip.heading = 135;
      break;
      case 135:
      myShip.heading = 180;
      break;
      case 180:
      myShip.heading = 225;
      break;
      case 225:
      myShip.heading = 270;
      break;
      case 270:
      myShip.heading = 315;
      break;
      case 315:
      myShip.heading = 0;
      break;
    };
  };
  
  if (gb.buttons.repeat(BUTTON_A,0)){
    myShip.mv.mag = 0;
    myShip.mv.dir = 0;
  };
  
  /*
  if (gb.buttons.repeat(BUTTON_B,0)){
    myShip.heading = (myShip.heading +10);
  };
  */
  myShip.draw();
  gb.display.println(myShip.x);
  gb.display.println(myShip.y);
  gb.display.println(myShip.heading);
  gb.display.println(myShip.mv.dir);
  gb.display.println(myShip.mv.getXpart());
  gb.display.println(myShip.mv.getYpart());
};
