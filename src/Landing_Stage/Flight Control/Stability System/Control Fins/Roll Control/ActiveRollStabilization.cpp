#include"ActiveRollStabilization.h"

 double *RPM_COMP = (double*)malloc(1);
 uint8_t CURRENT_SERVO_POSITION;

RollStability::RollStability(){
    //Constructor attaches servo motors to appropriate pins and retrieves
    //servo position from SD
    s1.attach(MOTOR_PIN_1);
    s2.attach(MOTOR_PIN_2);
    s3.attach(MOTOR_PIN_3);
    s4.attach(MOTOR_PIN_4);
    String search;
}

String RollStability::determineRollDirection(){
    //Roll direction can be calculated by using two roll values to
    //determine whether values follow a positive trend. If roll trend is negative
    //rocket rolls anticlockwise but if roll trend is positive rocket roll is cllockwise
    uint8_t sampleAmount = 3;
    double *rollV = compileValues(sampleAmount);
    uint8_t count;
    for(size_t i = 0; i < 2;i++){
        if(rollV[i] < rollV[i + 1]){
            count++;
        }    
    }
    if(count == 2){
      SPIN_DIRECTION = "anticlockwise"; 
    }
}

uint8_t RollStability::updateCurrentRollAxis(){
    //Update current Roll axis updates/stores roll axis. Roll axis:
    //Negative = (0 -> -179) 
    //Positive = (0 -> 179)
     uint8_t sampleAmount = 2;
    double *Val = compileValues(sampleAmount);
    if(Val[0] < 0){
        POS_ROLL_AXIS = false;
    }
}

double *RollStability::compileValues(uint8_t count){
    //Helper function to compile values
    Gyro rSen;
    double *vals = (double*)malloc(count);
    for(size_t i = 0;i < count;i++){
        vals[i] = rSen.AccGyroVals(1);
    }
    return vals;
}

double RollStability::getRollSpeed(){
    //We can get roll speed by dividing its roll distance(the difference between
    //roll points) with the roll time (the time it took to roll)
    double start,end,read1,read2;
    double time,dist;
    Gyro beg;
    read1 = beg.AccGyroVals(1);
    start = millis();
    read2 = beg.AccGyroVals(1);
    end = millis();
    time = (end - start) / 1000;
    dist = read2 - read1;
    return dist / time; // roll/sec
}

double RollStability::getRotationPM(){
    //Rocket RPM is computed by calculating the surface speed then 
    //using that to calculate rev per min
    double rSpeed = getRollSpeed();
    double diameter; //diameter in cm
    double diameter_cm_to_m,surfaceSpeed;
    //Calculate surfaceSpeed
    double rollCycle = 360 / rSpeed; //time it takes do do one rotation cycle at given speed
    diameter_cm_to_m = diameter / 100;
    surfaceSpeed = (pi * diameter_cm_to_m);
    surfaceSpeed = surfaceSpeed / rollCycle;
    //surfaceSpeed to RPM
    double ms_to_fm;//metres per second to feet per minute
    double diam_to_feet,RPM;
    ms_to_fm = surfaceSpeed * 196.85;
    RPM = ms_to_fm * 4;
    diam_to_feet = (diameter_cm_to_m * 39.37) / 12;
    RPM = RPM / diam_to_feet;
    return RPM;
}

uint8_t RollStability::computeCounterRoll(uint8_t RPM){
    //Function which takes the rocket current RPM and computes counter movement
    //for fins to stabilize roll
    double HR = updateHighestRPM();
    double servoThresh = 180;
    double temp1, temp2;
    temp1 = RPM_HIGH / servoThresh;
    temp2 = RPM / temp1;
    return temp2;
}

double RollStability::updateHighestRPM(){ //<-- Use more efficient implementation
    //Since the highest RPM in flight cannot be definetly measured, function constantly
    //retrieves and updates the highest RPM that has occured during flight. This is also used
    //to compute the counter movemet for the fins
    double RPM = getRotationPM();
    double highest = RPM_COMP[0];    
    //Add newRPM to array
    RPM_COMP[arSize - 1] = RPM;
    for(size_t i = 0; i < arSize;i++){
        if(RPM_COMP[i] > highest){
            highest = RPM_COMP[i];
        }
    }
    RPM_HIGH = highest;
    realloc(RPM_COMP,arSize + 1);
    arSize++;
    return;
}

void RollStability::finMovement(double x){
    //Function moves servos for rocket fins to pre-calculated counter position
    uint8_t DELAY_TIME;
    uint8_t count = CURRENT_SERVO_POSITION;
    uint8_t increment;
    if(count < x){
       increment = 1;
    }else if(count > x){
       increment = -1;
    }
    while(count != x){ 
       s1.write(count);
       delay(DELAY_TIME);
       s2.write(count);
       delay(DELAY_TIME);
       s3.write(count);
       delay(DELAY_TIME);
       s4.write(count);
       delay(DELAY_TIME);
       count += increment;
    }
}

 uint8_t RollStability::rollStabilize(uint8_t roll){
     //Driver function to stabilize vehicle roll
    uint8_t R_axis = 0; //Initialise to negative
    updateCurrentRollAxis();
    if(POS_ROLL_AXIS == true){
        R_axis = 1;  //If pos set to pos  
    }
    determineRollDirection();
    //PID control
    PROPORTIONAL_INTEGRAL_DERIVATIVE ax;
    Gyro read;
    double servoMov, referencePoint;
    double initkp,initki,initkd;
    ax.createPIDinstance("ROLL_STABILITY",initkp,initki,initkd);
    servoMov = ax.PID_MAIN("ROLL_STABILITY",read.AccGyroVals(1),referencePoint);
    finMovement(servoMov);
    return 1;
 }