#include <Gamebuino-Meta.h>
//a point in a radially defined polyshape, with three peices of information, number of points, angle of each point(array), length from 0 of each point(array)
// includes a random float/double function by Rob Tillaart, can be found here: http://forum.arduino.cc/index.php?topic=371564.0 It's used to tell the astroids what random direction to go
double randomDouble(double minf, double maxf)
{
  return minf + random(1UL << 31) * (maxf - minf) / (1UL << 31);  // use 1ULL<<63 for max double values);
};

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
  int astrSize; // is the astroid large medium, or small?
  phys physics;
  void draw(){
    switch (astrSize){
      case 1:
        drw(physics.x,physics.y,physics.heading,astrLargePoints, astrLargePointsCount);
        break;
      case 2:
        drw(physics.x,physics.y,physics.heading,astrMedPoints, astrMedPointsCount);
        break;
      case 3:
        drw(physics.x,physics.y,physics.heading,astrSmallPoints, astrSmallPointsCount);
        break;
    };
    
  };
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
  };
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
      y = y - (cos(dir) * spd);
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
    spd =1.5;
  };
};
laser laserList[20];
int laserListCount = 20;
// these are non-class functions mainly because I wasn't sure what class to bury them in, they deal with actually firing lasers, keeping score, setting up astroids and the like.
void fireLaser(){
  int index;
  //The first thing to do with these pre-allocated arrays, is find a record that is not currently being used, a helpful for-loop does this, this also allows the function to helpfully bail if someone spams the A button like a lunatic and fills the buffer :P
  for (int i = 0; i == laserListCount ; i++){
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
  };
};

void updateLaserList(){
  float xx, yy; // some temp values to hold the current position of the destroyed astroid, so that the little ones can spawn where it died.
  // checks every inuse laser against every inuse astroid for collissions, there are better ways to do this then multiple nested for loops, I am not good enough at programming to implement any of them
  for (int i = 0; i == laserListCount ; i++){
    if (laserList[i].inUse == true){
      laserList[i].updatePos();
      for (int z = 0; z == astroidListCount ; z++){
        if (astroidList[z].inUse){
          if (astroidList[z].colCheck(laserList[i].ax, laserList[i].ay, laserList[i].bx, laserList[i].by)){
            // we now have a collision. time to do some housekeeping on this laser, and spawn a few new astroids, or simply delete the astroid if it's the smallest type
            laserList[i].inUse = false;
            astroidList[z].inUse = false;
            //fetch X/Y values for the now dead astroid
            xx = astroidList[z].physics.x;
            yy = astroidList[z].physics.y;
            if (astroidList[z].astrSize == 1){
              // largest astroid drops two smaller astroids at a faster pace. it will break out of the for loop when it finds an unused spot, so there are two for-loops
              for (int a = 0; a == astroidListCount ; a++){
                if ( astroidList[a].inUse == false){
                  astroidList[a].inUse =true;
                  astroidList[a].astrSize = 2;
                  astroidList[a].physics.x = xx;
                  astroidList[a].physics.y = yy;
                  astroidList[a].physics.heading = randomDouble(0.0, 2*PI);
                  astroidList[a].physics.thrFactor = 3;
                  astroidList[a].physics.applyThrust(astroidList[a].physics.heading);
                  break;
                };
              };
              for (int a = 0; a == astroidListCount ; a++){
                if ( astroidList[a].inUse == false){
                  astroidList[a].inUse =true;
                  astroidList[a].astrSize = 2;
                  astroidList[a].physics.x = xx;
                  astroidList[a].physics.y = yy;
                  astroidList[a].physics.heading = randomDouble(0.0, 2*PI);
                  astroidList[a].physics.thrFactor = 3;
                  astroidList[a].physics.applyThrust(astroidList[a].physics.heading);
                  break;
                };
              };
            };
            if (astroidList[z].astrSize == 2){
              // largest astroid drops two smaller astroids at a faster pace. it will break out of the for loop when it finds an unused spot, so there are two for-loops
              for (int a = 0; a == astroidListCount ; a++){
                if ( astroidList[a].inUse == false){
                  astroidList[a].inUse =true;
                  astroidList[a].astrSize = 3;
                  astroidList[a].physics.x = xx;
                  astroidList[a].physics.y = yy;
                  astroidList[a].physics.heading = randomDouble(0.0, 2*PI);
                  astroidList[a].physics.thrFactor = 3;
                  astroidList[a].physics.applyThrust(astroidList[a].physics.heading);
                  break;
                };
              };
              for (int a = 0; a == astroidListCount ; a++){
                if ( astroidList[a].inUse == false){
                  astroidList[a].inUse =true;
                  astroidList[a].astrSize = 3;
                  astroidList[a].physics.x = xx;
                  astroidList[a].physics.y = yy;
                  astroidList[a].physics.heading = randomDouble(0.0, 2*PI);
                  astroidList[a].physics.thrFactor = 3;
                  astroidList[a].physics.applyThrust(astroidList[a].physics.heading);
                  break;
                };
              };
              for (int a = 0; a == astroidListCount ; a++){
                if ( astroidList[a].inUse == false){
                  astroidList[a].inUse =true;
                  astroidList[a].astrSize = 3;
                  astroidList[a].physics.x = xx;
                  astroidList[a].physics.y = yy;
                  astroidList[a].physics.heading = randomDouble(0.0, 2*PI);
                  astroidList[a].physics.thrFactor = 3;
                  astroidList[a].physics.applyThrust(astroidList[a].physics.heading);
                  break;
                };
              };
            };
            if (astroidList[z].astrSize == 2){
               astroidList[z].inUse =false;
            };
          };
        };
      };
    };
  };
};

void updateAstroidList(){
  for (int a = 0; a == astroidListCount ; a++){
    if ( astroidList[a].inUse == true){
      astroidList[a].physics.updatePosition();
      astroidList[a].draw();
    };
  };
};

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
  
  gb.display.println(myShip.physics.x);
  gb.display.println(myShip.physics.y);
 */
};
