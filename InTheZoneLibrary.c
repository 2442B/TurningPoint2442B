#pragma config(Sensor, in1,    baseLiftPoten,  sensorPotentiometer)
#pragma config(Sensor, in2,    topLiftPoten,   sensorPotentiometer)
#pragma config(Sensor, in4,    gyro,           sensorGyro)
#pragma config(Sensor, dgtl1,  leftQuad,       sensorQuadEncoder)
#pragma config(Sensor, dgtl3,  rightQuad,      sensorQuadEncoder)
#pragma config(Sensor, dgtl5,  forkliftButton, sensorDigitalIn)
#pragma config(Sensor, dgtl7,  centerPiston,   sensorDigitalOut)
#pragma config(Sensor, dgtl8,  redLED,         sensorLEDtoVCC)
#pragma config(Sensor, dgtl9,  yellowLED,      sensorLEDtoVCC)
#pragma config(Sensor, dgtl10, greenLED,       sensorLEDtoVCC)
#pragma config(Sensor, dgtl11, leftPiston,     sensorDigitalOut)
#pragma config(Sensor, dgtl12, rightPiston,    sensorDigitalOut)
#pragma config(Motor,  port1,           topLift,       tmotorVex393_HBridge, openLoop)
#pragma config(Motor,  port2,           baseLift,      tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port3,           driveLeftFront, tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port4,           driveLeftBack, tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port5,           driveRightFront, tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port6,           driveRightBack, tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port7,           mobileLiftLeft, tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port8,           mobileLiftRight, tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port9,           claw2,         tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port10,          claw,          tmotorVex393_HBridge, openLoop)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

#pragma systemFile

////GLOBAL VARIABLES////
//Poten Values For Lift -- Values increase as lift moves backwards
enum PotenValuesTop {BACK_TOP = 1000, UPRIGHT_TOP = 2508, MATCHLOAD_TOP = 580, SCORE_TOP = 4095};
enum PotenValuesClaw {BACK_CLAW = 3700, MATCHLOAD_CLAW = 750};
enum PotenValuesBase {BACK_BASE = 3875, MATCHLOAD_BASE = 3200, HIGHEST_BASE =  2608}; //values increase as lift moves down
int baseLiftPositions[12] = {3980,3980,3750,3700,3500,3350,3100,3400,3300,3250,2695,2525};

/*base
back = 3980
highest = 2423

topLift:
back = 355
up = 2608
scoreAll = 4095 (deadzone after)
*/
float BACK_KP_TOP = 1.7;
float MATCHLOAD_KP_TOP = 2;
float SCORE_KP_TOP = 2;
float BACK_KP_BASE = 10;
float SCORE_KP_BASE = 250	;
float MATCHLOAD_KP_BASE = 10;
int ERR_MARGIN = 50;

//for correctStraight task / driveStraight method
float rightPowerAdjustment = 0;
float leftPowerAdjustment = 0;
float theta = 0;

//for setTopLiftPos task / setTopLiftPos method
int desiredTop;
int powAfterTop;
float kpTop;
bool reachedMobileGoal = false;

//for setBaseLiftPos task / setBaseLiftPos method
int desiredBase;
int powAfterBase;
float kpBase;
int conesStacked = 0;

//for setClawUntilPos task / setClawUntilPos
int desiredClaw;
int clawPower;
bool userControlClaw = true;
bool userControlBase = true;

/////BASIC MOTOR METHODS/////
void setLeftMotors(int power)
{
	motor[driveLeftFront] = power;
	motor[driveLeftBack] = power;
}

void setRightMotors(int power)
{
	motor[driveRightFront] = power;
	motor[driveRightBack] = power;
}
void setAllDriveMotors(int power)
{
	setLeftMotors(power);
	setRightMotors(power);
}

void setTopLiftPower(int power)
{
	motor[topLift] = power;
	//motor[topLiftRight] = power;
}


void setBaseLiftPower(int power)
{
	motor[baseLift] = power;
	//motor[baseLiftRight] = power;
}


void setForkliftPower(int power)
{
	motor[mobileLiftLeft] = power;
	motor[mobileLiftRight]  = power;
}

void setClawPower(int power)
{
	motor[claw] = power;
	motor[claw2] = power;

}

/////////TASKS/////////
task correctStraight()
{
	rightPowerAdjustment = 0;
	leftPowerAdjustment = 0;
	int err = theta - SensorValue[gyro];
	int oldErr = err;
	int power;
	int deriv;
	int totalErr = err;
	int integral = totalErr;
	while(1)
	{
		err = theta - SensorValue[gyro];
		deriv = (err-oldErr)*0.5; //if error is increasing, apply more power (compensate for less momentum). else, apply more power
		integral = totalErr * 0.03;
		power = err*0.5 + deriv + integral;
		rightPowerAdjustment = power;
		leftPowerAdjustment = -power;
		oldErr = err;
		totalErr += err;
		//writeDebugStreamLine("Err: %d, Deriv: %d, TotalErr: %d, Integral: %d, Power: %d", err,deriv,totalErr,integral,power);
		wait1Msec(50);
	}
}

task setTopLiftPosTask() //reachedMobileGoal is only used in auton to stop and hold lift in place if robot reaches goal unexpectedly early
{
	bool ignore = false;
	if((desiredTop == BACK_TOP && SensorValue[topLiftPoten]<BACK_TOP) || reachedMobileGoal) //1000 = BACK value
		ignore = true;
	int err = desiredTop - SensorValue[topLiftPoten];
	int power = 127;

	while(abs(err)>200 &&  !ignore) //adjust power of motors while error is outide of certain range, then set power to 0
	{
		err = desiredTop - SensorValue[topLiftPoten];
		power = (int) (err*127/4095*kpTop);
		setTopLiftPower(power);
		//writeDebugStreamLine("Poten: %d, Power: %d, Error: %d", SensorValue[topLiftPoten], power,err);
		wait1Msec(50);
	}
	//writeDebugStreamLine("Moving to powAfter");
	setTopLiftPower(powAfterTop);
	if(reachedMobileGoal)
		setTopLiftPower(0);
}

task setBaseLiftPosTask()
{
	userControlBase = false;
	int err = desiredBase - SensorValue[baseLiftPoten];
	int power = 127;
	while(abs(err)>ERR_MARGIN) //adjust power of motors while error is outide of certain range, then set power to 0
	{
		err = desiredBase - SensorValue[baseLiftPoten];
		power = (int) (err*127/4095*kpBase);
		setBaseLiftPower(power);
		writeDebugStreamLine("Poten: %d, Power: %d, Error: %d", SensorValue[baseLiftPoten], power,err);
		wait1Msec(50);
	}
	setBaseLiftPower(powAfterBase);

}

task setClawUntilPosTask()
{
	userControlClaw = false;
	setClawPower(clawPower);
	while(SensorValue[topLiftPoten]>desiredClaw){} //wait until lift goes past a certain point moving from score to back
	setClawPower(-clawPower);
	wait1Msec(10);
	setClawPower(0);
	userControlClaw = true;
}


///////COMPLEX METHODS: a+bi///////
void driveStraight(int dest, int basePower, float rightMultiplier = 0.58) //uses correctStraight task (with gyro) to dive straight
{
	theta = SensorValue[gyro];
	SensorValue[leftQuad] = 0;
	SensorValue[rightQuad] = 0;
	int err = dest;
	int power = 127;
	startTask(correctStraight);
	//writeDebugStreamLine("err: %d, power: %d sdfdgdsgfgfsggffs",err,power);
	while(fabs(err)>20 && fabs(dest - (-1*SensorValue[rightQuad]))>20)
	{
		err = dest - SensorValue[leftQuad];
		power = basePower*sgn(err);
		setRightMotors((int)(power*rightMultiplier + rightPowerAdjustment));
		setLeftMotors((int) (power+leftPowerAdjustment));
		wait1Msec(50);
		//writeDebugStreamLine("RightAdjustment: %d, LeftAdjustment: %d", rightPowerAdjustment, leftPowerAdjustment);
		//writeDebugStreamLine("err: %d, power: %d, rpower: %d",err,power,(int)(power*rightMultiplier + rightPowerAdjustment));
	}
	stopTask(correctStraight);
	setAllDriveMotors(0);
}

void turnToPos(int pos)
{
	clearTimer(T4);
	int err = pos - SensorValue[gyro];
	int power;
	while(fabs(err) > 80 && time1(T4)<2500)
	{
		err = pos - SensorValue[gyro];
		power = 127*err*0.004+10;
		setRightMotors(power);
		setLeftMotors(-power);
		wait1Msec(50);
	}
}

void setTopLiftPos(int aDesired, float aKp, int aPowAfter = 0)
{
	reachedMobileGoal = false;
	desiredTop = aDesired;
	kpTop = aKp;
	powAfterTop = aPowAfter;
	startTask(setTopLiftPosTask);
}

void setBaseLiftPos(int aDesired, float aKp, int aPowAfter = 0)
{
	reachedMobileGoal = false;
	desiredBase = aDesired;
	kpBase = aKp;
	powAfterBase = aPowAfter;
	startTask(setBaseLiftPosTask);
}

void setClawUntilPos(int aDesiredClaw, int aClawPower)
{
	desiredClaw = aDesiredClaw;
	clawPower = aClawPower;
	startTask(setClawUntilPosTask);
}

/////MORE COMPLEX TASKS///
task autoScoreTask()
{
	int potenConstant = 100;
	//writeDebugStreamLine("cones stacked when in method: %d, baseLiftPos: %d", conesStacked);
	//writeDebugStreamLine(" baseLiftPos: %d", 	baseLiftPositions[conesStacked]);
	setBaseLiftPos(baseLiftPositions[conesStacked],SCORE_KP_BASE);
	writeDebugStreamLine("must be less than this level: %d", baseLiftPositions[conesStacked] - potenConstant - ERR_MARGIN);
	while(SensorValue[baseLiftPoten] > baseLiftPositions[conesStacked] + potenConstant + ERR_MARGIN){}
	setTopLiftPos(SCORE_TOP,SCORE_KP_TOP);
	conesStacked++;
}

task autoBackTask()
{
	userControlClaw = false;
	if(SensorValue[topLiftPoten]>UPRIGHT_TOP)
		setClawPower(-80); //open claw
	setTopLiftPos(BACK_TOP,BACK_KP_TOP);
	while(SensorValue[topLiftPoten]>UPRIGHT_TOP){}
	setBaseLiftPos(BACK_BASE,BACK_KP_BASE,0);
	setClawPower(0); //close claw
}

////AND CORRESPONDING MORE COMPLEX METHODS/////
void autoScore()
{
	startTask(autoScoreTask);
	wait1Msec(3000);
}

void autoBack()
{
	startTask(autoBackTask);
}
