#include <Gamebuino-Meta.h>
//a point in a radially defined polyshape, with three peices of information, number of points, angle of each point(array), length from 0 of each point(array)
// includes a random float/double function by Rob Tillaart, can be found here: http://forum.arduino.cc/index.php?topic=371564.0 It's used to tell the astroids what random direction to go

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
bool onMenu = false;

double randomDouble(double minf, double maxf)
{
  return minf + random(1UL << 31) * (maxf - minf) / (1UL << 31);  // use 1ULL<<63 for max double values);
};


// to encode a shape, start at 0 radians, and each new point has a radian value and a length value, it is always closed by the drawing class
poly shipPoints[] = {{0.0,6.0},{3.9269,4.5},{3.145,0.75},{2.3562,4.5}};
const int shipPointCount = 4; // how many points are in this shape
//poly astrLargePoints[] = {{‪0.785398‬, 9}, {1.570796‬, 9}, {2.356194‬, 9}, {3.141592‬, 9}, {3.92699‬, 9}, {4.712388‬, 9}, {5.497786‬, 9}, {6.283184‬, 9}};
poly astrLargePoints[] = {{0.7854,9}, {1.5708,9}, {2.3542,9}, {3.1416,9}, {3.927,9}, {4.7124,9}, {5.498,9}, {6.2831,9}};
const int astrLargePointsCount = 8;
//poly astrMedPoints[] = {{‪0.785398‬, 4.5}, {1.570796‬, 4.5}, {2.356194‬, 4.5}, {3.141592‬, 4.5}, {3.92699‬, 4.5}, {4.712388‬,4.5}, {5.497786‬, 4.5}, {6.283184‬, 4.5}};
poly astrMedPoints[] = {{0.7854,4.5}, {1.5708,4.5}, {2.3542,4.5}, {3.1416,4.5}, {3.927,4.5}, {4.7124,4.5}, {5.498,4.5}, {6.2831,4.5}};
const int astrMedPointsCount = 8;
//poly astrSmallPoints[] = {{‪0.785398‬, 2.5}, {1.570796‬, 2.5}, {2.356194‬, 2.5}, {3.141592‬, 2.5}, {3.92699‬, 2.5}, {4.712388‬, 2.5}, {5.497786‬, 2.5}, {6.283184‬, 2.5}};
poly astrSmallPoints[] = {{0.7854,2.5}, {1.5708,2.5}, {2.3542,2.5}, {3.1416,2.5}, {3.927,2.5}, {4.7124,2.5}, {5.498,2.5}, {6.2831,2.5}};
const int astrSmallPointsCount = 8;
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
// collision detection class based on the geeks for geeks method of line segment checking
// internal struct for points used for collision detection
struct xypoint {
  float x,y;
  xypoint(float xx, float yy){
    x = xx;
    y = yy;
  };
  xypoint(){
    x = 0;
    y = 0;
  };
};
class intersect{
  private:
  xypoint pA,qA,pB,qB;
  bool segment(xypoint p, xypoint q, xypoint r){
    if (q.x <= max(p.x, r.x) && q.x >= min(p.x, r.x) && q.y <= max(p.y, r.y) && q.y >= min(p.y, r.y)){
      return true;
    } else {
      return false;
    }; 
  };
  
  int orient(xypoint p, xypoint q, xypoint r){
    int val = (q.y - p.y) * (r.x - q.x) - (q.x - p.x) * (r.y - q.y);

    if (val == 0) return 0; // colinear

    return (val > 0)? 1: 2; // clock or counterclock wise
  };
  bool doIntercept(){
    // Find the four orientations needed for general and
    // special cases
    int oA = orient(pA, qA, pB);
    int oB = orient(pA, qA, qB);
    int oC = orient(pB, qB, pA);
    int oD = orient(pB, qB, qA);
    
    if (oA != oB && oC != oD){
       return true;
    };

    //colinear cases, which require some extra checking.
    if (oA == 0 && segment(pA, pB, qA)){
      return true;
    };
    if (oB == 0 && segment(pA, qB, qA)){
      return true;
    };
    if (oC == 0 && segment(pB, pA, qB)){
      return true;
    };
    if (oD == 0 && segment(pB, qA, qB)){
      return true;
    };

    return false; // no intersect, so in this game's case, no collision
  };
  public:
  bool check(float ax, float ay, float bx, float by, float cx, float cy, float dx, float dy){
    // load variables from parts of the game never inteded to work in this way.
    pA.x = ax;
    pA.y = ay;
    qA.x = bx;
    qA.y = by;
    pB.x = cx;
    pB.y = cy;
    qB.x = dx;
    qB.y = dy;
    //return the output from the private side of the function, this prevents any accidental breakage of the function by some other part of the program, it's overkill for Meteor, but I wanted to learn how to use Public and Private.
    return doIntercept();
  };
};
// an instince of the intersect class
intersect inter;
// a special draw function to deal with the radially encouded 'vector' graphics and collision detection.
void drw(float posx, float posy, float dir, poly shape[], int pointCount){
  //gb.display.drawLine(gb.display.width() / 2, gb.display.height() / 2,(gb.display.width() / 2) + sin(shipDir) * 10, (gb.display.height() / 2) + cos(shipDir) * 10);
  float xa,ya,xb,yb,xs,ys  = 0.0;
  bool redraw = false;
  //dir -= PI; //for reasons somewhat beyond me, it draws things upside down, this is an attempt to patch taht.
  for(int i=0; i <= pointCount; i++){
    //special case for first point in the shape, since a single point line doesn't work
    if (i == 0){
      //calclates what the x and y should be, based on the angle and radius of the point, and the x, y, and angle of the entity being drawn
      xa = posx + (sin(dir+shape[i].getA()) * shape[i].getR());
      ya = posy + (cos(dir+shape[i].getA()) * shape[i].getR());
      // this section detects if the first point is out of bounds, and tells the function to draw a line shifted appropriately to the other side.
      if (xa < 0 || xa  > gb.display.width()){
        if (xa < 0 ) {
          xs = gb.display.width(); 
        } else {
          xs = -gb.display.width(); 
        };
        redraw = true;
      };
      if (ya < 0 || ya > gb.display.height()){
        if (ya < 0 ) {
          ys = gb.display.height(); 
        } else {
          ys = -gb.display.height(); 
        };
        redraw = true;
      };
    };
    if (i >0 && i < (pointCount)){
      // now we have a start point, and are now getting the next point, drawing the line, and pushing those values to the xa/ya ones to repeat
      xb = posx + (sin(dir+shape[i].getA()) * shape[i].getR());
      yb = posy + (cos(dir+shape[i].getA()) * shape[i].getR());
      // cancles the 'redraw' mode for lines that are not out of bounds on either point

      if (xb  < 0 || xb  > gb.display.width()){
        if (xb < 0 ) {
          xs = gb.display.width(); 
        } else {
          xs = -gb.display.width(); 
        };
        redraw = true;
      };
      if (yb  < 0 || yb  > gb.display.height()){
        if (yb < 0 ) {
          ys = gb.display.height() ; 
        } else {
          ys = -gb.display.height(); 
        };
        redraw = true;
      };
      if ((xa  >= 0 && xa  <= gb.display.width()) && (xb  >= 0 && xb  <= gb.display.width())){
        if ((ya  >= 0 && ya  <= gb.display.height()) && (yb  >= 0 && yb  <= gb.display.height())){
          redraw = false;
        };
      };
      //is the line completely out of bounds (and naturally being drawn by the redraw part of this function anyway)?, if not, draw this line.
      if (!((xa  <= 0 || xa  >= gb.display.width()) && (xb  <= 0 || xb  >= gb.display.width()))){
        if (!((ya  <= 0 || ya  >= gb.display.height()) && (yb  <= 0 || yb  >= gb.display.height()))){
          gb.display.drawLine(xa,ya,xb,yb);
        };
      };
      if (redraw) {
        gb.display.drawLine(xa+xs,ya+ys,xb+xs,yb+ys);
      };
      xa = xb;
      ya = yb;
    };
    if (i == (pointCount)){
      //we are on the last datapoint, we will need to get this information from the first point to close the polygon
      xb = posx + (sin(dir+shape[0].getA()) * shape[0].getR());
      yb = posy + (cos(dir+shape[0].getA()) * shape[0].getR());
      if (xb  < 0 || xb > gb.display.width()){
        if (xb < 0 ) {
          xs = gb.display.width(); 
        } else {
          xs = - gb.display.width(); 
        };
        redraw = true;
      };
      if (yb  < 0 || yb > gb.display.height()){
        if (yb < 0 ) {
          ys = gb.display.height(); 
        } else {
          ys = -gb.display.height(); 
        };
        redraw = true;
      };
      if ((xa  >= 0 && xa  <= gb.display.width()) && (xb  >= 0 && xb  <= gb.display.width())){
        if ((ya  >= 0 && ya  <= gb.display.height()) && (yb  >= 0 && yb  <= gb.display.height())){
          redraw = false;
        };
      };
      //draw the flipped line
      if (!((xa  <= 0 || xa  >= gb.display.width()) && (xb  <= 0 || xb  >= gb.display.width()))){
        if (!((ya  <= 0 || ya  >= gb.display.height()) && (yb  <= 0 || yb  >= gb.display.height()))){
          gb.display.drawLine(xa,ya,xb,yb);
        };
      };
      if (redraw) {
        gb.display.drawLine(xa+xs,ya+ys,xb+xs,yb+ys);
      };
    };
  };
};
// this is a version of the drawing function that doesn't draw, but instead checks a shape for any collissions. seperated from the drawing function for load reasons.
/*
bool collision(float posx, float posy, float dir, poly shape[], int pointCount, float segAX, float segAY,float segBX, float segBY){
  //gb.display.drawLine(gb.display.width() / 2, gb.display.height() / 2,(gb.display.width() / 2) + sin(shipDir) * 10, (gb.display.height() / 2) + cos(shipDir) * 10);
  float xa,ya,xb,yb,xs,ys  = 0.0;
  bool redraw = false;
  //bool collide = false;
  //dir -= PI; //for reasons somewhat beyond me, it draws things upside down, this is an attempt to patch taht.
  for(int i=0; i <= pointCount; i++){
    //special case for first point in the shape, since a single point line doesn't work
    if (i == 0){
      //calclates what the x and y should be, based on the angle and radius of the point, and the x, y, and angle of the entity being drawn
      xa = posx + (sin(dir+shape[i].getA()) * shape[i].getR());
      ya = posy + (cos(dir+shape[i].getA()) * shape[i].getR());
      // this section detects if the first point is out of bounds, and tells the function to draw a line shifted appropriately to the other side.
      if (xa < 0 || xa  > gb.display.width()){
        if (xa < 0 ) {
          xs = gb.display.width(); 
        } else {
          xs = -gb.display.width(); 
        };
        redraw = true;
      };
      if (ya < 0 || ya > gb.display.height()){
        if (ya < 0 ) {
          ys = gb.display.height(); 
        } else {
          ys = -gb.display.height(); 
        };
        redraw = true;
      };
    };
    if (i >0 && i < (pointCount)){
      // now we have a start point, and are now getting the next point, drawing the line, and pushing those values to the xa/ya ones to repeat
      xb = posx + (sin(dir+shape[i].getA()) * shape[i].getR());
      yb = posy + (cos(dir+shape[i].getA()) * shape[i].getR());
      // cancles the 'redraw' mode for lines that are not out of bounds on either point

      if (xb  < 0 || xb  > gb.display.width()){
        if (xb < 0 ) {
          xs = gb.display.width(); 
        } else {
          xs = -gb.display.width(); 
        };
        redraw = true;
      };
      if (yb  < 0 || yb  > gb.display.height()){
        if (yb < 0 ) {
          ys = gb.display.height() ; 
        } else {
          ys = -gb.display.height(); 
        };
        redraw = true;
      };
      if ((xa  >= 0 && xa  <= gb.display.width()) && (xb  >= 0 && xb  <= gb.display.width())){
        if ((ya  >= 0 && ya  <= gb.display.height()) && (yb  >= 0 && yb  <= gb.display.height())){
          redraw = false;
        };
      };
      //is the line completely out of bounds (and naturally being drawn by the redraw part of this function anyway)?, if not, draw this line.
      if (!((xa  <= 0 || xa  >= gb.display.width()) && (xb  <= 0 || xb  >= gb.display.width()))){
        if (!((ya  <= 0 || ya  >= gb.display.height()) && (yb  <= 0 || yb  >= gb.display.height()))){
          //gb.display.drawLine(xa,ya,xb,yb);
          if (inter.check(xa,ya,xb,yb,segAX,segAY,segBX,segBY)){
             return true;
          };
        };
      };
      if (redraw) {
        //gb.display.drawLine(xa+xs,ya+ys,xb+xs,yb+ys);
        if (inter.check(xa+xs,ya+ys,xb+xs,yb+ys,segAX,segAY,segBX,segBY)){
             return true;
        };
      };
      xa = xb;
      ya = yb;
    };
    if (i == (pointCount)){
      //we are on the last datapoint, we will need to get this information from the first point to close the polygon
      xb = posx + (sin(dir+shape[0].getA()) * shape[0].getR());
      yb = posy + (cos(dir+shape[0].getA()) * shape[0].getR());
      if (xb  < 0 || xb > gb.display.width()){
        if (xb < 0 ) {
          xs = gb.display.width(); 
        } else {
          xs = - gb.display.width(); 
        };
        redraw = true;
      };
      if (yb  < 0 || yb > gb.display.height()){
        if (yb < 0 ) {
          ys = gb.display.height(); 
        } else {
          ys = -gb.display.height(); 
        };
        redraw = true;
      };
      if ((xa  >= 0 && xa  <= gb.display.width()) && (xb  >= 0 && xb  <= gb.display.width())){
        if ((ya  >= 0 && ya  <= gb.display.height()) && (yb  >= 0 && yb  <= gb.display.height())){
          redraw = false;
        };
      };
      //draw the flipped line
      if (!((xa  <= 0 || xa  >= gb.display.width()) && (xb  <= 0 || xb  >= gb.display.width()))){
        if (!((ya  <= 0 || ya  >= gb.display.height()) && (yb  <= 0 || yb  >= gb.display.height()))){
          //gb.display.drawLine(xa,ya,xb,yb);
          if (inter.check(xa+xs,ya+ys,xb+xs,yb+ys,segAX,segAY,segBX,segBY)){
             return true;
          };
        };
      };
      if (redraw) {
        //gb.display.drawLine(xa+xs,ya+ys,xb+xs,yb+ys);
        if (inter.check(xa+xs,ya+ys,xb+xs,yb+ys,segAX,segAY,segBX,segBY)){
             return true;
        };
      };
    };
  };
  return false;
};
*/
//Ship class, doesn't need any position variables or anything like that, since it's part of the physics class the ship inherits
class ship{
  public:
  phys physics; // all of the elements of the class above can be called by ship.physics.<variable/function>
  void draw(){
    drw(physics.x,physics.y,physics.heading,shipPoints, shipPointCount);
  };
};
class astroid{
  public:
  bool inUse; // whether this astroid is in use or not
  int astrSize, invincible; // is the astroid large medium, or small?
  phys physics;
  void draw(){
    switch (astrSize){
      case 1:
        //drw(physics.x,physics.y,physics.heading,astrLargePoints, astrLargePointsCount);
        gb.display.drawCircle(physics.x,physics.y, 9);
        gb.display.setCursor(physics.x,physics.y);
        //gb.display.print("1");
        break;
      case 2:
        //drw(physics.x,physics.y,physics.heading,astrMedPoints, astrMedPointsCount);
        gb.display.drawCircle(physics.x,physics.y, 4.5);
       //gb.display.setCursor(physics.x,physics.y);
        //.display.print("2");
        break;
      case 3:
        //drw(physics.x,physics.y,physics.heading,astrSmallPoints, astrSmallPointsCount);
        gb.display.drawCircle(physics.x,physics.y, 2.5);
        //gb.display.setCursor(physics.x,physics.y);
        //gb.display.print("3");
        break;
    };
    
  };/*
  bool colCheck(float lx,float ly, float mx, float my){
    switch (astrSize){
      case 1:
        return collision(physics.x,physics.y,physics.heading,astrLargePoints, astrLargePointsCount, lx, ly, mx, my);
        break;
      case 2:
        return collision(physics.x,physics.y,physics.heading,astrMedPoints, astrMedPointsCount, lx, ly, mx, my);
        break;
      case 3:
        return collision(physics.x,physics.y,physics.heading,astrSmallPoints, astrSmallPointsCount, lx, ly, mx, my);
        break;
      default:
        return false;
    };
    //return collision(physics.x,physics.y,physics.heading,shipPoints, shipPointCount, lx, ly, mx, my);
  };*/
  // these are constructors, special functions named the same as the class that allow you to declare an instance of the function with some starting values. the second of the two covers the case for no starting values being given.
  astroid(bool use, int sze){
    inUse = use;
    astrSize = sze;
  };
  astroid(){
    inUse = 0;
    astrSize = 1;
    
  };
};
//because of the defualt constructor, setting up an array with the appropriate allocation of memory is possible without defining everything
astroid astroidList[18];
int astroidListCount = 18;
// of course, we'll only need one player spacecraft instance
ship myShip;
// lazors! Well, the class and array that stores them, anyway
class laser{
  public:
  float ax,ay,bx,by; // x/y of endpoints for the short line segment that is a laser bolt
  bool inUse; // whether this spot is in use.
  float x, y, spd, dir;
  int timer; // kill the laser when the timer hits 0
  void updatePos(){
    // simplified physics for these guys, since they don't change direction or anything fancy.
    // also updates endpoints
      x = x + (sin(dir) * spd);
      y = y + (cos(dir) * spd);
      ax = x + (sin(dir+PI) * 1.5);
      ay = y + (cos(dir+PI) * 1.5);
      bx = x + (sin(dir-PI) * 1.5);
      by = y + (cos(dir-PI) * 1.5);
      //return true;
  };
  void draw(){
    gb.display.drawLine(ax,ay,bx,by);
  };
  laser(){
    inUse = false;
    timer = 100;
    spd =5.5;
  };
};
laser laserList[20];
int laserListCount = 20;
// these are non-class functions mainly because I wasn't sure what class to bury them in, they deal with actually firing lasers, keeping score, setting up astroids and the like.
void fireLaser(){
  int index;
  //The first thing to do with these pre-allocated arrays, is find a record that is not currently being used, a helpful for-loop does this, this also allows the function to helpfully bail if someone spams the A button like a lunatic and fills the buffer :P
  for (int i = 0; i < laserListCount -1 ; i++){
    if (laserList[i].inUse == false){
      index = i;
      break;
    };
    index = -1;
  };
  if (index != -1){
    // we have a valid useable index, let's actually fire the laser
    laserList[index].x = myShip.physics.x;
    laserList[index].y = myShip.physics.y;
    laserList[index].dir = myShip.physics.heading;
    laserList[index].inUse = true;
    laserList[index].timer = 0;
  };
};

void updateLaserList(){
  int asize;
  float xx, yy; // some temp values to hold the current position of the destroyed astroid, so that the little ones can spawn where it died.
  // checks every inuse laser against every inuse astroid for collissions, there are better ways to do this then multiple nested for loops, I am not good enough at programming to implement any of them
  for (int i = 0; i < laserListCount ; i++){
    if (laserList[i].inUse == true){
      laserList[i].timer +=1;
      laserList[i].updatePos();
      laserList[i].draw();
      if (laserList[i].x < 0 || laserList[i].x > gb.display.width()){
        laserList[i].inUse = false;
      };
      if (laserList[i].y < 0 || laserList[i].y > gb.display.height()){
        laserList[i].inUse = false;
      };
      if (laserList[i].timer > 40){
        laserList[i].inUse = false;
        laserList[i].timer = 0;
      };
      for (int z = 0; z < astroidListCount ; z++){
        if (astroidList[i].inUse = true){
          switch (astroidList[z].astrSize){
          case 1:
          asize = 9;
          break;
          case 2:
          asize = 4.5;
          break;
          case 3:
          asize = 2.5;
          break;
        };
        if (((sqrt(sq(astroidList[z].physics.x-laserList[i].x)+ sq(astroidList[z].physics.y-laserList[i].y)) <= asize+1) && laserList[i].inUse != false) && astroidList[z].invincible == 0)   {
            if (astroidList[z].physics.xVel + astroidList[z].physics.yVel == 0) {
              laserList[i].inUse = false;
              laserList[i].timer = 0;
              astroidList[z].inUse = false;
            } else {
              // we've now determined that we've had a hit, that the astroid isn't invicible to prevent muti-strikes, and that the laser isn't out of bounds.
              // spawm smaller astroids, first copy the physics state of the current rock, so that x/y can be moved to it's children, and then clear the velocities
              laserList[i].inUse = false;
              laserList[i].timer = 0;
              astroidList[z].inUse = false;
              xx = astroidList[z].physics.x;
              yy = astroidList[z].physics.y;
              astroidList[z].physics.x = 0;
              astroidList[z].physics.y = 0;
              astroidList[z].physics.xVel = 0;
              astroidList[z].physics.yVel = 0;
              astroidList[z].physics.thrFactor = 1;
              astroidList[z].invincible = 15;
              switch (astroidList[z].astrSize){
                case 1: // big astroid, spits two mediums
                for (int i = 0; i < 2 ; i++) {
                  for (int a = 0; a < astroidListCount ; a++){
                    if ( astroidList[a].inUse == false){
                      astroidList[a].inUse = true;
                      astroidList[a].astrSize = 2;
                      astroidList[a].physics.x = xx;
                      astroidList[a].physics.y = yy;
                      astroidList[a].physics.xVel = 0;
                      astroidList[a].physics.yVel = 0;
                      astroidList[a].invincible = 15;
                      astroidList[a].physics.heading = randomDouble(0.0, 2*PI);
                      astroidList[a].physics.thrFactor = 0.5;
                      astroidList[a].physics.applyThrust(astroidList[a].physics.heading);
                      gb.display.print(a);
                      break;
                    };
                  };
                };
                break;
                case 2:
                for (int i = 0; i < 3 ; i++) {
                  for (int a = 0; a < astroidListCount ; a++){
                   if ( astroidList[a].inUse == false){
                      astroidList[a].inUse = true;
                      astroidList[a].astrSize = 3;
                      astroidList[a].physics.x = xx;
                      astroidList[a].physics.y = yy;
                      astroidList[a].physics.xVel = 0;
                      astroidList[a].physics.yVel = 0;
                      astroidList[a].invincible = 10;
                      astroidList[a].physics.heading = randomDouble(0.0, 2*PI);
                      astroidList[a].physics.thrFactor = 0.5;
                      astroidList[a].physics.applyThrust(astroidList[a].physics.heading);
                      break;
                    };
                  };
                };
                break;
                case 3:
                //gb.display.print("Hit 3");
                break;
              };
            };          
          } else {
            if (((sqrt(sq(astroidList[z].physics.x-laserList[i].x)+ sq(astroidList[z].physics.y-laserList[i].y)) <= asize+1) && laserList[i].inUse != false) && astroidList[z].invincible != 0) {
              laserList[i].inUse = false;
              laserList[i].timer = 0;
            };
          };
        };
      };
    };
  };
};
void updateAstroidList(){
  for (int a = 0; a < astroidListCount ; a++){
    // if this astroid isn't moving, it's bugged, kill it before it mustates.
      if (astroidList[a].physics.xVel + astroidList[a].physics.yVel == 0){
        astroidList[a].inUse = false;
      }
    if ( astroidList[a].inUse == true){     
      astroidList[a].physics.updatePosition();
      astroidList[a].draw();
      if ( astroidList[a].invincible != 0){
        astroidList[a].invincible -= 1;
      };
    };
  };
};
int stage = 0; // a game is best served with a difficulty ramp, this game is no different, this controls what stage you're on, and the overaching gameplay
bool stageReady = false; // gives the gameplay a pause to clear the screen, display the current stage, and reset things.
int stagebreaktimer = 0;
/* BUTTON constants on the META are an enumerated Class. What that means for us, is we can set up extra variables not tied to actual buttons, but *game functions*,
and then have a function call that changes what button constant that calls to. This is called Mapping. You could theoretically do this with ANY button,
but out of respect to the design language, only the four d-pad buttons and a/b are abstracted this way. This allows our main loop to only care about that the 
'fire laser' button has been pressed, regardless if that's A, B or one of the D-pad buttons, with a function that changes these values, we have the ability to map controls! 
To make sure things work out of the box, we set the defualt scheme*/
Button BUTTON_ROTATELEFT =  BUTTON_LEFT;
Button BUTTON_ROTATERIGHT =  BUTTON_RIGHT;
Button BUTTON_FORWARDTHRUST =  BUTTON_UP;
Button BUTTON_RETROTHRUST =  BUTTON_DOWN;
Button BUTTON_FIRE =  BUTTON_A;
bool isdead; //you died, how sad. plays a tune and restarts the game.
// main brannch of the menu
const char* mainMenuItems[] = {
  "Back to Game",
  "Controls",
  "Colors",
  "Sounds",
};

const char* ColorsMenuItems[] = {
  "Back to Options",
  "Space",
  "Ship",
  "Astroids",
  "lasers",
  "Text"
};

const char* ColorsMenuOptions[] = {
  "Return to Colors",
  "WHITE",
  "GRAY",
  "DARKGRAY",
  "BLACK",
  "PURPLE",
  "PINK",
  "RED",
  "ORANGE",
  "BROWN",
  "BEIGE",
  "YELLOW",
  "LIGHTGREEN",
  "GREEN",
  "DARKBLUE",
  "BLUE",
  "LIGHTBLUE",
};

const char* ControlsMenuItems[] = {
  "Back to Options",
  "Turn Left",
  "Turn Right",
  "Forward",
  "Reverse",
  "Fire"
};
const char* ControlsMenuOptions[] = {
  "Back to Options",
  "UP",
  "DOWN",
  "LEFT",
  "RIGHT",
  "A",
  "B"
};
const char* endCredits[18] = {
"Congratulations!",
"METEOR: V0.8",
"Written By:",
"DFX2KX",
"Special Thanks:",
"Rodot & META Team",
"Riksu9000",
"Geeks for Geeks",
"Arduino Community"
};
const char* altCredits[19] = {
"METEOR: V0.8",
"Written By:",
"DFX2KX",
"Special Thanks:",
"Rodot & META Team",
"Riksu9000",
"Geeks for Geeks",
"Arduino Community"
};
Color COLOR_SHIP = WHITE;
Color COLOR_ASTROID = DARKGRAY;
Color COLOR_LASER = BLUE;
Color COLOR_SPACE = BLACK; // Spaaaaaaaceeee! It's the background color.
Color COLOR_TEXT = WHITE; //text color

void doEnd(int mode){
  /*
  // end credits

    if (mode){
      for (int i = 0; i=18; i++){
        gb.display.println(endCredits[i]);
      };
    } else {
    for (int i = 0; i=19; i++){
        gb.display.println(altCredits[i]);
      };
    };
    */
};
void doMenu() {
// The Menu-nest of the game.
int SelectMain, SelectSub, SelectSub2 = 0;// where you are in the nest, tracks what menu to show when you back out of one
bool exitMenu = false; // thing that differentiates whether you selected the 'back to game option, or are just sitting at the ready
// prepare for lots and lots of while loops and ifs
while (!exitMenu){
  // make sure we're not in a submenu when refreshing
  if (!(SelectSub != 0 && SelectSub2 != 0)){
    SelectMain = gb.gui.menu("Options:", mainMenuItems);
  };  
  if (SelectMain != 0) {
    if (SelectMain == 1) {
      SelectSub = gb.gui.menu("Controls:", ControlsMenuItems);
      if (SelectSub != 0) {
        // user has selected a control to change! Time to do that!
        if (SelectSub == 1) {
          // User has selected 'Turn left'
          SelectSub2 = gb.gui.menu("Turn Left:", ControlsMenuOptions);
          if (SelectSub2 !=0) {
            switch (SelectSub2){
              case 1: // UP
                BUTTON_ROTATELEFT = BUTTON_UP; // reassingment
                break;
              case 2: // DOWN
                BUTTON_ROTATELEFT = BUTTON_DOWN; // reassingment
                break;
              case 3: // LEFT
                BUTTON_ROTATELEFT = BUTTON_LEFT; // reassingment
                break;
              case 4: // RIGHT
                BUTTON_ROTATELEFT =  BUTTON_RIGHT; // reassingment
                break;
              case 5: // A
                BUTTON_ROTATELEFT = BUTTON_A; // reassingment
                break;
              case 6: // B
                BUTTON_ROTATELEFT =  BUTTON_B; // reassingment
                break;
            };
            SelectSub2 = 0; // return to controls Menu after selection
            SelectSub = gb.gui.menu("Controls:", ControlsMenuItems);
          } else {
            SelectSub2 = 0; // set conditions for entering the controls menu
            SelectSub = gb.gui.menu("Controls:", ControlsMenuItems);
          };
        };
        if (SelectSub == 2) {
          // User has selected 'Turn Right'
          SelectSub2 = gb.gui.menu("Turn Right:", ControlsMenuOptions);
          if (SelectSub2 !=0) {
            switch (SelectSub2){
              case 1: // UP
                BUTTON_ROTATERIGHT =  BUTTON_UP; // reassingment
                break;
              case 2: // DOWN
                BUTTON_ROTATERIGHT =  BUTTON_DOWN; // reassingment
                break;
              case 3: // LEFT
                BUTTON_ROTATERIGHT =  BUTTON_LEFT; // reassingment
                break;
              case 4: // RIGHT
                BUTTON_ROTATERIGHT =  BUTTON_RIGHT; // reassingment
                break;
              case 5: // A
                BUTTON_ROTATERIGHT =  BUTTON_A; // reassingment
                break;
              case 6: // B
                BUTTON_ROTATERIGHT =  BUTTON_B; // reassingment
                break;
            };
            SelectSub2 = 0; // return to controls Menu after selection
            SelectSub = gb.gui.menu("Controls:", ControlsMenuItems);
          } else {
            SelectSub2 = 0; // set conditions for entering the controls menu
            SelectSub = gb.gui.menu("Controls:", ControlsMenuItems);
          };
        };
        if (SelectSub == 3) {
          // User has selected 'Forward'
          SelectSub2 = gb.gui.menu("Forward:", ControlsMenuOptions);
          if (SelectSub2 !=0) {
            switch (SelectSub2){
              case 1: // UP
                BUTTON_FORWARDTHRUST =  BUTTON_UP; // reassingment
                break;
              case 2: // DOWN
                BUTTON_FORWARDTHRUST =  BUTTON_DOWN; // reassingment
                break;
              case 3: // LEFT
                BUTTON_FORWARDTHRUST =  BUTTON_LEFT; // reassingment
                break;
              case 4: // RIGHT
                BUTTON_FORWARDTHRUST =  BUTTON_RIGHT; // reassingment
                break;
              case 5: // A
                BUTTON_FORWARDTHRUST =  BUTTON_A; // reassingment
                break;
              case 6: // B
                BUTTON_FORWARDTHRUST =  BUTTON_B; // reassingment
                break;
            };
            SelectSub2 = 0; // return to controls Menu after selection
            SelectSub = gb.gui.menu("Controls:", ControlsMenuItems);
          } else {
            SelectSub2 = 0; // set conditions for entering the controls menu
            SelectSub = gb.gui.menu("Controls:", ControlsMenuItems);
          };
        };
        if (SelectSub == 4) {
          // User has selected 'Reverse'
          SelectSub2 = gb.gui.menu("Reverse:", ControlsMenuOptions);
          if (SelectSub2 !=0) {
            switch (SelectSub2){
              case 1: // UP
                BUTTON_FORWARDTHRUST =  BUTTON_UP; // reassingment
                break;
              case 2: // DOWN
                BUTTON_FORWARDTHRUST =  BUTTON_DOWN; // reassingment
                break;
              case 3: // LEFT
                BUTTON_FORWARDTHRUST =  BUTTON_LEFT; // reassingment
                break;
              case 4: // RIGHT
                BUTTON_FORWARDTHRUST =  BUTTON_RIGHT; // reassingment
                break;
              case 5: // A
                BUTTON_FORWARDTHRUST =  BUTTON_A; // reassingment
                break;
              case 6: // B
                BUTTON_FORWARDTHRUST =  BUTTON_B; // reassingment
                break;
            };
            SelectSub2 = 0; // return to controls Menu after selection
            SelectSub = gb.gui.menu("Controls:", ControlsMenuItems);
          } else {
            SelectSub2 = 0; // set conditions for entering the controls menu
            SelectSub = gb.gui.menu("Controls:", ControlsMenuItems);
          };
        };
        if (SelectSub == 5) {
          // User has selected 'Firese'
          SelectSub2 = gb.gui.menu("Fire:", ControlsMenuOptions);
          if (SelectSub2 !=0) {
            switch (SelectSub2){
              case 1: // UP
                BUTTON_FIRE =  BUTTON_UP; // reassingment
                break;
              case 2: // DOWN
                BUTTON_FIRE =  BUTTON_DOWN; // reassingment
                break;
              case 3: // LEFT
                BUTTON_FIRE =  BUTTON_LEFT; // reassingment
                break;
              case 4: // RIGHT
                BUTTON_FIRE =  BUTTON_RIGHT; // reassingment
                break;
              case 5: // A
                BUTTON_FIRE =  BUTTON_A; // reassingment
                break;
              case 6: // B
                BUTTON_FIRE =  BUTTON_B; // reassingment
                break;
            };
            SelectSub2 = 0; // return to controls Menu after selection
            SelectSub = gb.gui.menu("Controls:", ControlsMenuItems);
          } else {
            SelectSub2 = 0; // set conditions for entering the controls menu
            SelectSub = gb.gui.menu("Controls:", ControlsMenuItems);
          };
        };
      } else {
        // user has backed out to main menu
        SelectSub = 0; // set submenu to zero so it returns to initial top level state
        SelectMain = gb.gui.menu("Options:", mainMenuItems);
      };
    };
    if (SelectMain == 2) {
      SelectSub = gb.gui.menu("Colors:", ColorsMenuItems);
      if (SelectSub != 0){
        if (SelectSub == 1) {
          // User has selected 'Space'
          SelectSub2 = gb.gui.menu("Space:", ColorsMenuOptions);
          if (SelectSub2 !=0) {
            switch (SelectSub2){
              case 1: // UP
                COLOR_SPACE = WHITE; // reassingment
                break;
              case 2: // DOWN
                COLOR_SPACE = GRAY; // reassingment
                break;
              case 3: // LEFT
                COLOR_SPACE = DARKGRAY; // reassingment
                break;
              case 4: // RIGHT
                COLOR_SPACE =  BLACK; // reassingment
                break;
              case 5: // A
                COLOR_SPACE = PURPLE; // reassingment
                break;
              case 6: // B
                COLOR_SPACE =  PINK; // reassingment
                break;
              case 7: // B
                COLOR_SPACE =  RED; // reassingment
                break;
              case 8: // B
                COLOR_SPACE =  ORANGE; // reassingment
                break;
              case 9: // B
                COLOR_SPACE =  BROWN; // reassingment
                break;
              case 10: // B
                COLOR_SPACE =  BEIGE; // reassingment
                break;
              case 11: // B
                COLOR_SPACE =  YELLOW; // reassingment
                break;
              case 12: // B
                COLOR_SPACE =  LIGHTGREEN; // reassingment
                break;
              case 13: // B
                COLOR_SPACE =  GREEN; // reassingment
                break;
              case 14: // B
                COLOR_SPACE =  DARKBLUE; // reassingment
                break;
              case 15: // B
                COLOR_SPACE =  BLUE; // reassingment
                break;
              case 16: // B
                COLOR_SPACE =  LIGHTBLUE; // reassingment
                break;
            };
            SelectSub2 = 0; // return to controls Menu after selection
            SelectSub = gb.gui.menu("Colors:", ColorsMenuItems);
          } else {
            SelectSub2 = 0; // set conditions for entering the controls menu
            SelectSub = gb.gui.menu("Colors:", ColorsMenuItems);
          };
        };
        if (SelectSub == 2) {
          // User has selected 'Space'
          SelectSub2 = gb.gui.menu("Ship:", ColorsMenuOptions);
          if (SelectSub2 !=0) {
            switch (SelectSub2){
              case 1: // UP
                COLOR_SHIP = WHITE; // reassingment
                break;
              case 2: // DOWN
                COLOR_SHIP = GRAY; // reassingment
                break;
              case 3: // LEFT
                COLOR_SHIP = DARKGRAY; // reassingment
                break;
              case 4: // RIGHT
                COLOR_SHIP =  BLACK; // reassingment
                break;
              case 5: // A
                COLOR_SHIP = PURPLE; // reassingment
                break;
              case 6: // B
                COLOR_SHIP =  PINK; // reassingment
                break;
              case 7: // B
                COLOR_SHIP =  RED; // reassingment
                break;
              case 8: // B
                COLOR_SHIP =  ORANGE; // reassingment
                break;
              case 9: // B
                COLOR_SHIP =  BROWN; // reassingment
                break;
              case 10: // B
                COLOR_SHIP =  BEIGE; // reassingment
                break;
              case 11: // B
                COLOR_SHIP =  YELLOW; // reassingment
                break;
              case 12: // B
                COLOR_SHIP =  LIGHTGREEN; // reassingment
                break;
              case 13: // B
                COLOR_SHIP =  GREEN; // reassingment
                break;
              case 14: // B
                COLOR_SHIP =  DARKBLUE; // reassingment
                break;
              case 15: // B
                COLOR_SHIP =  BLUE; // reassingment
                break;
              case 16: // B
                COLOR_SHIP =  LIGHTBLUE; // reassingment
                break;
            };
            SelectSub2 = 0; // return to controls Menu after selection
            SelectSub = gb.gui.menu("Colors:", ColorsMenuItems);
          } else {
            SelectSub2 = 0; // set conditions for entering the controls menu
            SelectSub = gb.gui.menu("Colors:", ColorsMenuItems);
          };
        };
        
        if (SelectSub == 3) {
          // User has selected 'Space'
          SelectSub2 = gb.gui.menu("Astroids:", ColorsMenuOptions);
          if (SelectSub2 !=0) {
            switch (SelectSub2){
              case 1: // UP
                COLOR_ASTROID = WHITE; // reassingment
                break;
              case 2: // DOWN
                COLOR_ASTROID = GRAY; // reassingment
                break;
              case 3: // LEFT
                COLOR_ASTROID = DARKGRAY; // reassingment
                break;
              case 4: // RIGHT
                COLOR_ASTROID =  BLACK; // reassingment
                break;
              case 5: // A
                COLOR_ASTROID = PURPLE; // reassingment
                break;
              case 6: // B
                COLOR_ASTROID =  PINK; // reassingment
                break;
              case 7: // B
                COLOR_ASTROID =  RED; // reassingment
                break;
              case 8: // B
                COLOR_ASTROID =  ORANGE; // reassingment
                break;
              case 9: // B
                COLOR_ASTROID =  BROWN; // reassingment
                break;
              case 10: // B
                COLOR_ASTROID =  BEIGE; // reassingment
                break;
              case 11: // B
                COLOR_ASTROID =  YELLOW; // reassingment
                break;
              case 12: // B
                COLOR_ASTROID =  LIGHTGREEN; // reassingment
                break;
              case 13: // B
                COLOR_ASTROID =  GREEN; // reassingment
                break;
              case 14: // B
                COLOR_ASTROID =  DARKBLUE; // reassingment
                break;
              case 15: // B
                COLOR_ASTROID =  BLUE; // reassingment
                break;
              case 16: // B
                COLOR_ASTROID =  LIGHTBLUE; // reassingment
                break;
            };
            SelectSub2 = 0; // return to controls Menu after selection
            SelectSub = gb.gui.menu("Colors:", ColorsMenuItems);
          } else {
            SelectSub2 = 0; // set conditions for entering the controls menu
            SelectSub = gb.gui.menu("Colors:", ColorsMenuItems);
          };
        };
        if (SelectSub == 4) {
          // User has selected 'Space'
          SelectSub2 = gb.gui.menu("Lasers:", ColorsMenuOptions);
          if (SelectSub2 !=0) {
            switch (SelectSub2){
              case 1: // UP
                COLOR_LASER = WHITE; // reassingment
                break;
              case 2: // DOWN
                COLOR_LASER = GRAY; // reassingment
                break;
              case 3: // LEFT
                COLOR_LASER = DARKGRAY; // reassingment
                break;
              case 4: // RIGHT
                COLOR_LASER =  BLACK; // reassingment
                break;
              case 5: // A
                COLOR_LASER = PURPLE; // reassingment
                break;
              case 6: // B
                COLOR_LASER =  PINK; // reassingment
                break;
              case 7: // B
                COLOR_LASER =  RED; // reassingment
                break;
              case 8: // B
                COLOR_LASER =  ORANGE; // reassingment
                break;
              case 9: // B
                COLOR_LASER =  BROWN; // reassingment
                break;
              case 10: // B
                COLOR_LASER =  BEIGE; // reassingment
                break;
              case 11: // B
                COLOR_LASER =  YELLOW; // reassingment
                break;
              case 12: // B
                COLOR_ASTROID =  LIGHTGREEN; // reassingment
                break;
              case 13: // B
                COLOR_LASER =  GREEN; // reassingment
                break;
              case 14: // B
                COLOR_LASER =  DARKBLUE; // reassingment
                break;
              case 15: // B
                COLOR_LASER =  BLUE; // reassingment
                break;
              case 16: // B
                COLOR_LASER =  LIGHTBLUE; // reassingment
                break;
            };
            SelectSub2 = 0; // return to controls Menu after selection
            SelectSub = gb.gui.menu("Colors:", ColorsMenuItems);
          } else {
            SelectSub2 = 0; // set conditions for entering the controls menu
            SelectSub = gb.gui.menu("Colors:", ColorsMenuItems);
          };
        };
        if (SelectSub == 5) {
          // User has selected 'Space'
          SelectSub2 = gb.gui.menu("Text:", ColorsMenuOptions);
          if (SelectSub2 !=0) {
            switch (SelectSub2){
              case 1: // UP
                COLOR_TEXT = WHITE; // reassingment
                break;
              case 2: // DOWN
                COLOR_TEXT = GRAY; // reassingment
                break;
              case 3: // LEFT
                COLOR_TEXT = DARKGRAY; // reassingment
                break;
              case 4: // RIGHT
                COLOR_TEXT =  BLACK; // reassingment
                break;
              case 5: // A
                COLOR_TEXT = PURPLE; // reassingment
                break;
              case 6: // B
                COLOR_TEXT =  PINK; // reassingment
                break;
              case 7: // B
                COLOR_TEXT =  RED; // reassingment
                break;
              case 8: // B
                COLOR_TEXT =  ORANGE; // reassingment
                break;
              case 9: // B
                COLOR_LASER =  BROWN; // reassingment
                break;
              case 10: // B
                COLOR_TEXT =  BEIGE; // reassingment
                break;
              case 11: // B
                COLOR_TEXT =  YELLOW; // reassingment
                break;
              case 12: // B
                COLOR_TEXT =  LIGHTGREEN; // reassingment
                break;
              case 13: // B
                COLOR_TEXT =  GREEN; // reassingment
                break;
              case 14: // B
                COLOR_TEXT =  DARKBLUE; // reassingment
                break;
              case 15: // B
                COLOR_TEXT =  BLUE; // reassingment
                break;
              case 16: // B
                COLOR_TEXT =  LIGHTBLUE; // reassingment
                break;
            };
            SelectSub2 = 0; // return to controls Menu after selection
            SelectSub = gb.gui.menu("Colors:", ColorsMenuItems);
          } else {
            SelectSub2 = 0; // set conditions for entering the controls menu
            SelectSub = gb.gui.menu("Colors:", ColorsMenuItems);
          };
        };
      } else {
        SelectSub = 0; // set submenu to zero so it returns to initial top level state
        SelectMain = gb.gui.menu("Options:", ControlsMenuItems);
      };
    };
    if (SelectMain == 3) {
      // coming soon: Sounds
      SelectSub = 0; // set submenu to zero so it returns to initial top level state
        SelectMain = gb.gui.menu("Options:", ControlsMenuItems);
    };
    if (SelectMain == 4) {
      // coming soon High Score
      
      SelectSub = 0; // set submenu to zero so it returns to initial top level state
        SelectMain = gb.gui.menu("Options:", ControlsMenuItems);
    };
    if (SelectMain == 5) {
      // coming soon credits
      doEnd(0);
      SelectSub = 0; // set submenu to zero so it returns to initial top level state
        SelectMain = gb.gui.menu("Options:", ControlsMenuItems);
    };
  } else{
    
    exitMenu = true; // back to the game
  };
  
};

};


// and so is color. so, guess what, we're getting customizable colors, too! Good for folks who want inverted contrast, or go full on rainbow.
bool settingsSave() {
  int a = 0;
  a += gb.save.set(0, BUTTON_ROTATELEFT);
  a += gb.save.set(1, BUTTON_ROTATERIGHT);
  a += gb.save.set(2, BUTTON_FORWARDTHRUST);
  a += gb.save.set(3, BUTTON_RETROTHRUST);
  a += gb.save.set(4, BUTTON_FIRE);
  a += gb.save.set(5, COLOR_SHIP);
  a += gb.save.set(6, COLOR_ASTROID);
  a += gb.save.set(7, COLOR_LASER);
  a += gb.save.set(8, COLOR_SPACE);
  a += gb.save.set(9, COLOR_TEXT);
  if (a ==10){
   return true;
  } else {
    return false;
  };
};
bool settingsLoad() {
    int a = 0;
  a += gb.save.get(0, BUTTON_ROTATELEFT);
  a += gb.save.get(1, BUTTON_ROTATERIGHT);
  a += gb.save.get(2, BUTTON_FORWARDTHRUST);
  a += gb.save.get(3, BUTTON_RETROTHRUST);
  a += gb.save.get(4, BUTTON_FIRE);
  a += gb.save.get(5, COLOR_SHIP);
  a += gb.save.get(6, COLOR_ASTROID);
  a += gb.save.get(7, COLOR_LASER);
  a += gb.save.get(8, COLOR_SPACE);
  a += gb.save.get(9, COLOR_TEXT);
  if (a ==10){
   return true;
  } else {
    return false;
  };
};
bool doStage(int nextstage) {
  // this readies the game for the next stage. sticks up the number of the stage, and animates the ship icon
  gb.display.clear();
  gb.display.fill(COLOR_SPACE);
   for (int a = 0; a != astroidListCount ; a++){
   astroidList[a].inUse =false;
   astroidList[a].physics.xVel = 0;
   astroidList[a].physics.yVel = 0;
   }; // just to make sure no mess is somehow left over
   for (int i = 0; i != laserListCount ; i++){
    laserList[i].inUse = false;
   };
   myShip.physics.x = gb.display.width()/2;
   myShip.physics.y = gb.display.height()/2;
   myShip.physics.xVel = 0;
   myShip.physics.yVel = 0;
   myShip.physics.heading = 0;
  switch (nextstage){
    // configure the asteroids at the start of the stage
    case 0: // welcome to the tutorial level.
      astroidList[0].inUse =true;
      astroidList[0].astrSize = 3;
      astroidList[0].physics.x = gb.display.width() + 8;
      astroidList[0].physics.y = gb.display.height() + 12;
      astroidList[0].physics.heading = randomDouble(0.0, 2*PI);
      astroidList[0].physics.thrFactor = 0.5;
      astroidList[0].physics.applyThrust(astroidList[0].physics.heading);
      astroidList[1].inUse =true;
      astroidList[1].astrSize = 3;
      astroidList[1].physics.x = gb.display.width() + 10;
      astroidList[1].physics.y = gb.display.height() + 12;
      astroidList[1].physics.heading = randomDouble(0.0, 2*PI);
      astroidList[1].physics.thrFactor = 0.5;
      astroidList[1].physics.applyThrust(astroidList[0].physics.heading);
    break;
    case 1: // welcome to the tutorial level.
      astroidList[0].inUse =true;
      astroidList[0].astrSize = 2;
      astroidList[0].physics.x = gb.display.width()+ 8;
      astroidList[0].physics.y = gb.display.height()+ 5;
      astroidList[0].physics.heading = randomDouble(0.0, 2*PI);
      astroidList[0].physics.thrFactor = .5;
      astroidList[0].physics.applyThrust(astroidList[0].physics.heading);
    break;
    case 2: // welcome to the tutorial level.
      astroidList[0].inUse =true;
      astroidList[0].astrSize = 1;
      astroidList[0].physics.x = gb.display.width() + 6;
      astroidList[0].physics.y = gb.display.height() + 9;
      astroidList[0].physics.heading = randomDouble(0.0, 2*PI);
      astroidList[0].physics.thrFactor = .5;
      astroidList[0].physics.applyThrust(astroidList[0].physics.heading);
    break;
    case 3: // welcome to the tutorial level.
      astroidList[0].inUse =true;
      astroidList[0].astrSize = 2;
      astroidList[0].physics.x = gb.display.width() + 6;
      astroidList[0].physics.y = gb.display.height() + 9;
      astroidList[0].physics.heading = randomDouble(0.0, 2*PI);
      astroidList[0].physics.thrFactor = .5;
      astroidList[0].physics.applyThrust(astroidList[0].physics.heading);
      astroidList[1].inUse =true;
      astroidList[1].astrSize = 2;
      astroidList[1].physics.x = gb.display.width() - 6;
      astroidList[1].physics.y = gb.display.height() + 12;
      astroidList[1].physics.heading = randomDouble(0.0, 2*PI);
      astroidList[1].physics.thrFactor = .5;
      astroidList[1].physics.applyThrust(astroidList[0].physics.heading);
      astroidList[2].inUse =true;
      astroidList[2].astrSize = 2;
      astroidList[2].physics.x = gb.display.width() - 2;
      astroidList[2].physics.y = gb.display.height() -2;
      astroidList[2].physics.heading = randomDouble(0.0, 2*PI);
      astroidList[2].physics.thrFactor = .5;
      astroidList[2].physics.applyThrust(astroidList[0].physics.heading);
    break;
    case 4: // welcome to the tutorial level.
      astroidList[0].inUse =true;
      astroidList[0].astrSize = 1;
      astroidList[0].physics.x = gb.display.width() - 10;
      astroidList[0].physics.y = gb.display.height() + 9;
      astroidList[0].physics.heading = randomDouble(0.0, 2*PI);
      astroidList[0].physics.thrFactor = .5;
      astroidList[0].physics.applyThrust(astroidList[0].physics.heading);
      astroidList[1].inUse =true;
      astroidList[1].astrSize = 1;
      astroidList[1].physics.x = gb.display.width() + 15;
      astroidList[1].physics.y = gb.display.height() + 1;
      astroidList[1].physics.heading = randomDouble(0.0, 2*PI);
      astroidList[1].physics.thrFactor = .5;
      astroidList[1].physics.applyThrust(astroidList[0].physics.heading);
    break;
    case 5: // welcome to the tutorial level.
      astroidList[0].inUse =true;
      astroidList[0].astrSize = 1;
      astroidList[0].physics.x = gb.display.width() + 1;
      astroidList[0].physics.y = gb.display.height() + 30;
      astroidList[0].physics.heading = randomDouble(0.0, 2*PI);
      astroidList[0].physics.thrFactor = .5;
      astroidList[0].physics.applyThrust(astroidList[0].physics.heading);
      astroidList[1].inUse =true;
      astroidList[1].astrSize = 2;
      astroidList[1].physics.x = gb.display.width() + 6;
      astroidList[1].physics.y = gb.display.height() - 9;
      astroidList[1].physics.heading = randomDouble(0.0, 2*PI);
      astroidList[1].physics.thrFactor = 1;
      astroidList[1].physics.applyThrust(astroidList[0].physics.heading);
      astroidList[2].inUse =true;
      astroidList[2].astrSize = 2;
      astroidList[2].physics.x = gb.display.width() + 6;
      astroidList[2].physics.y = gb.display.height() - 24;
      astroidList[2].physics.heading = randomDouble(0.0, 2*PI);
      astroidList[2].physics.thrFactor = 1;
      astroidList[2].physics.applyThrust(astroidList[0].physics.heading);
    break;
    case 6:
      //doEnd(1);
      stage = 0;
      break;
  };
  return true;
};
// And here we are at the functions where the menu configures things.
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
  stage = 0;
  stageReady = 0;
  if (!settingsLoad()) {
    // there was an issue loading settings data, or there is no sav file with that data in it, load defaults
    COLOR_SHIP = WHITE;
    COLOR_ASTROID = DARKGRAY;
    COLOR_LASER = BLUE;
    COLOR_SPACE = BLACK;
    COLOR_TEXT = WHITE; 
    BUTTON_ROTATELEFT =  BUTTON_LEFT;
    BUTTON_ROTATERIGHT =  BUTTON_RIGHT;
    BUTTON_FORWARDTHRUST =  BUTTON_UP;
    BUTTON_RETROTHRUST =  BUTTON_DOWN;
    BUTTON_FIRE =  BUTTON_A;
    gb.gui.popup(" Using Defaults.", 50);                  
  } else {
    gb.gui.popup("Settings loaded!", 50);
  };
};
int endcreditloop = 0;
int wipetimer = 15;
int remaining = 0;
//int ascana, ascanb = 0; // diagnosing the bug of doom.
void loop() {
  // put your main code here, to run repeatedly:
  while (!gb.update());
  gb.display.clear();
  if (onMenu){
    if(gb.buttons.repeat(BUTTON_MENU, 0)){
      // Menu has been hit, go back into the game
      doMenu();
      if (!settingsSave()){
        gb.gui.popup("Save error!", 50);
      } else {
        gb.gui.popup("Settings saved!", 50);
      };
      onMenu = false;
    };
  } else {  
      // main game loop
      
      if (!stageReady) {
          
          //stage trasitions and housekeeping go in here.
          if (isdead) {
            gb.display.println("      Game Over  ");
            if (endcreditloop == 0) {
              gb.display.setCursor(0,0);
              endcreditloop +=1;
            };
            if (endcreditloop < 150) {
                gb.display.clear();
                gb.display.fill(COLOR_SPACE);
                gb.display.setColor(COLOR_TEXT);
                gb.display.println("      Game Over  ");
                endcreditloop +=1;
            };
            if (endcreditloop >= 150) {
              stage = 0;
              stageReady = 0;
              endcreditloop = 0;
              doStage(0);
              isdead = false;
            };  
          } else {
            if (stage == 6) {
           if (endcreditloop == 0) {
            gb.display.setCursor(0,0);
            endcreditloop +=1;
           };
           if (endcreditloop >= 350) {
            stage = 0;
            stageReady = 0;
            endcreditloop =0;
            doStage(0);
           };
           if (endcreditloop < 350) {
              gb.display.clear();
              gb.display.fill(COLOR_SPACE);
              gb.display.setColor(COLOR_TEXT);
              // draw the appropriate version of the credits at the right spot
              // gb.display.setCursorY(gb.display.height() - endcreditloop *.25);
              for (int i = 0; i!=18; i++) {
                  gb.display.println(endCredits[i]);
               };
              endcreditloop +=1;
           };
          } else {
          gb.display.clear();
          gb.display.fill(COLOR_SPACE);
          gb.display.setColor(COLOR_TEXT);
          gb.display.println(" ");
          gb.display.print("    Stage: ");
          gb.display.print(stage);
          gb.display.println("!");
          gb.display.setColor(COLOR_SHIP);
          myShip.physics.x = gb.display.width()/2;
          myShip.physics.y = gb.display.height()/2;
          myShip.physics.heading += 0.087;
          gb.display.setColor(COLOR_SHIP);
          myShip.draw();
           stagebreaktimer +=1;
           if (stagebreaktimer == 50){
           stageReady = doStage(stage);
           stagebreaktimer = 0;
           };
          }; 
          };        
        } else {
        // you're now in a stage, normal gameplay applies
        if(gb.buttons.repeat(BUTTON_ROTATELEFT, 0)){
          myShip.physics.heading += 0.12568;
        };
        if(gb.buttons.repeat(BUTTON_ROTATERIGHT, 0)){
          myShip.physics.heading -= 0.12568;
        };
        if(gb.buttons.repeat(BUTTON_FORWARDTHRUST, 0)){
          myShip.physics.applyThrust(myShip.physics.heading);
        };
        if(gb.buttons.repeat(BUTTON_RETROTHRUST, 0)){
          myShip.physics.applyThrust(myShip.physics.heading-PI);
        };
        if(gb.buttons.pressed(BUTTON_FIRE)){
          fireLaser();
        };
        if(gb.buttons.repeat(BUTTON_MENU, 0)){
          // Menu has been hit, go into menu mode
          onMenu = true;
        };
        gb.display.fill(COLOR_SPACE);
        myShip.physics.applyDrag(myShip.physics.drgFactor);
        myShip.physics.updatePosition();
        gb.display.setColor(COLOR_SHIP);
        myShip.draw();
        gb.display.setColor(COLOR_ASTROID);
        updateAstroidList();
        gb.display.setColor(COLOR_LASER);
        updateLaserList();
        // checking for all astroids distroyed, handle getting smacked by one.
        remaining = 0;
        for (int a = 0; a != astroidListCount ; a++) {
          if (astroidList[a].inUse) {
            remaining +=1;
            int asize;
              switch (astroidList[a].astrSize) {
              case 1:
              asize = 9;
              break;
              case 2:
              asize = 4.5;
              break;
              case 3:
              asize = 2.5;
              break;
              };
                if (sqrt(sq(astroidList[a].physics.x-myShip.physics.x)+ sq(astroidList[a].physics.y-myShip.physics.y)) <= asize+2) {
                  isdead = true;
                  stageReady = false;
                  remaining = 1;
                  wipetimer = 15;
                  endcreditloop = 0;
                };
            };
        };
        if (remaining == 0) {
          wipetimer -=1;
        };
        if (wipetimer == 0) {
          stage +=1;
          stageReady = false;
          remaining = 1;
          wipetimer= 15;
        } else {
          if (remaining !=0 ) {
            wipetimer = 15;
          };
        };
        
        };
      };
   
};
