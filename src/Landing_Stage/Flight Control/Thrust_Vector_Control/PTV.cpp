#include"PTV.h"

double PredictedThrustVectoring::Rate_Of_Change_Estimate(uint8_t DOF){
    double *samples = (double*)malloc(3);
    double startTime = millis();
    for(size_t i = 0; i < 2;i++){
        samples[i] = vGet.AccGyroVals(DOF);
    }
    double endTime = millis();
    double ROC = (samples[2] - samples[0]) / (endTime - startTime);
    return ROC;
}

double PredictedThrustVectoring::eq_of_Motion_estimate(uint8_t DOF){
    //Final pos = initial pos + initial vel + 0.5(acceleration * time**2)
    //Final velocity = initial velocity + (acceleration * time)
    double initialPos = vGet.AccGyroVals(DOF);
    double initialVel = Rate_Of_Change_Estimate(DOF);
    double acceleration;
    double time;
    double finalPos = (initialPos + initialVel) + 0.5 * (acceleration * pow(time,2));
    return finalPos;

}

double PredictedThrustVectoring::final_pitch_estimate(){
    //Use kalman filter to get pitch estimate at t + BTdelay
    //where(t) = Nav TVC command time
    //Get Measurement Error
    double mError;
    //Get State Estimate Error
    double sEst = eq_of_Motion_estimate(2);
    //Set process noise 
    double pNoise;
    //init k filter
    SimpleKalmanFilter pitchFilter(mError,sEst,pNoise);
    //Estimate
    double finalEst = posEst.updateEstimate(sEst);
    return finalEst;
}

double PredictedThrustVectoring::final_yaw_estimate(){
    //Get Measurement Error
    double measureError;
    //Get State Estimate Error
    double stateEst = eq_of_Motion_estimate(2);
    //Set process noise 
    double prNoise;
    //init k filter
    SimpleKalmanFilter pitchFilter(measureError,stateEst,prNoise);
    //Estimate
    double finalEst = posEst.updateEstimate(stateEst);
    return finalEst;
}

double PredictedThrustVectoring::computeMotorVector(double x){

}