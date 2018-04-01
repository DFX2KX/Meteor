#include <Gamebuino-Meta.h>
//a point in a radially defined polyshape, with three peices of information, number of points, angle of each point(array), length from 0 of each point(array)
class poly {
  private:
  float a,r; // angle and radius of any particular point
  public:
  poly(float ang, float rad){
    // contstructor to allow for initialization of pot
    a = ang;
    r = rad;
  };
  float getA(){
    return a;
  };
  float getR(){
    return r;
  };
};
// to encode a shape, start at 0 radians, and each new point has a radian value and a length value, it is always closed by the drawing class
//poly shipPoints[] = {{0.0,10.0},{2.3562,4.0},{3.145,0.75},{3.9269,4.0}};
poly shipPoints[] = {{0.0,6.0},{3.9269,4.5},{3.145,0.75},{2.3562,4.5}};
const int shipPointCount = 4; // how many points are in this shape

// a 'Physics' class to implement the excellent method demonstrated by Riksu9000 as the physics method for any class that requires it
class phys {
   public:
   //variables for: location, velocity, acceleration, and direction
   float x, y, xVel,yVel, xThr, yThr, thrFactor, drgFactor, heading;
   void updatePosition(){
    // using temp variables to reduce function calls a bit
    int w = gb.display.width();
    int h = gb.display.height();
      x += xVel;
      y += yVel;

      if (x < 0){
        x = w;
      };
      if (x > w){
        x = 0;
      };
      if (y < 0){
        y = h;
      };
      if (y > h){
        y = 0;
      };
   };
   void applyThrust(float dir){
      xThr = sin(dir) * thrFactor;
      yThr = cos(dir) * thrFactor;

      xVel += xThr;
      yVel += yThr;
   };
   void applyDrag(float drg){
      xVel *= drg;
      yVel *= drg;
   };
   void setRotation(float rate){
      //positve rate rotates counter clockwise
      heading = heading + rate;
   };
   
};
// a special draw function to deal with the radially encouded 'vector' graphics and collision detection.
void drw(float posx, float posy, float dir, poly shape[], int pointCount){
  //gb.display.drawLine(gb.display.width() / 2, gb.display.height() / 2,(gb.display.width() / 2) + sin(shipDir) * 10, (gb.display.height() / 2) + cos(shipDir) * 10);
  float xa,ya,xb,yb;
  //dir -= PI; //for reasons somewhat beyond me, it draws things upside down, this is an attempt to patch taht.
  for(int i=0; i <= pointCount; i++){
    //special case for first point in the shape, since a single point line doesn't work
    if (i == 0){
      //calclates what the x and y should be, based on the angle and radius of the point, and the x, y, and angle of the entity being drawn
      xa = posx + (sin(dir+shape[i].getA()) * shape[i].getR());
      ya = posy + (cos(dir+shape[i].getA()) * shape[i].getR());
    };
    if (i >0 && i < (pointCount)){
      // now we have a start point, and are now getting the next point, drawing the line, and pushing those values to the xa/ya ones to repeat
      xb = posx + (sin(dir+shape[i].getA()) * shape[i].getR());
      yb = posy + (cos(dir+shape[i].getA()) * shape[i].getR());
      gb.display.drawLine(xa,ya,xb,yb);
      xa = xb;
      ya = yb;
    };
    if (i == (pointCount)){
      //we are on the last datapoint, we will need to get this information from the first point to close the polygon
      xb = posx + (sin(dir+shape[0].getA()) * shape[0].getR());
      yb = posy + (cos(dir+shape[0].getA()) * shape[0].getR());
      gb.display.drawLine(xa,ya,xb,yb);
    };
  };
};
//Ship class, doesn't need any position variables or anything like that, since it's part of the physics class the ship inherits
class ship{
  public:
  phys physics; // all of the elements of the class above can be called by ship.physics.<variable/function>
  void draw(){
    drw(physics.x,physics.y,physics.heading,shipPoints, shipPointCount);
  };
};
ship myShip;

void setup() {
  // put your setup code here, to run once:
  gb.begin();
  myShip.physics.x = gb.display.width()/2;
  myShip.physics.y = gb.display.height()/2;
  myShip.physics.thrFactor =0.2;
  myShip.physics.drgFactor =0.96;
  myShip.physics.xVel = 0.0;
  myShip.physics.yVel = 0.0;
  myShip.physics.heading = 0.0;
}

void loop() {
  // put your main code here, to run repeatedly:
  while (!gb.update());
  gb.display.clear();
  //myShip.draw();
  if(gb.buttons.repeat(BUTTON_LEFT, 0)){
    myShip.physics.heading += 0.12568;
  };
  if(gb.buttons.repeat(BUTTON_RIGHT, 0)){
    myShip.physics.heading -= 0.12568;
  };
  if(gb.buttons.repeat(BUTTON_UP, 0)){
    myShip.physics.applyThrust(myShip.physics.heading);
  };
  myShip.physics.applyDrag(myShip.physics.drgFactor);
  myShip.physics.updatePosition();
  myShip.draw();
  /*
  gb.display.setColor(GREEN);
  gb.display.drawLine(gb.display.width() / 2, gb.display.height() / 2,(gb.display.width() / 2) + sin(myShip.physics.heading) * 10, (gb.display.height() / 2) + cos(myShip.physics.heading) * 10);
  gb.display.setColor(WHITE);
  */
  gb.display.println(myShip.physics.xVel);
  gb.display.println(myShip.physics.yVel);
  gb.display.println(myShip.physics.x);
  gb.display.println(myShip.physics.y);
 
};
