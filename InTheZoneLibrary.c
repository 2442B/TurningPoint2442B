#pragma config(Sensor, in1,    topLiftPoten,   sensorPotentiometer)
#pragma config(Sensor, in2,    baseLiftPoten,  sensorPotentiometer)
#pragma config(Sensor, in4,    gyro,           sensorGyro)
#pragma config(Sensor, dgtl1,  rightQuad,      sensorQuadEncoder)
#pragma config(Sensor, dgtl5,  forkliftButton, sensorDigitalIn)
#pragma config(Sensor, dgtl8,  redLED,         sensorLEDtoVCC)
#pragma config(Sensor, dgtl9,  yellowLED,      sensorLEDtoVCC)
#pragma config(Sensor, dgtl10, greenLED,       sensorLEDtoVCC)
#pragma config(Sensor, dgtl11, leftQuad,       sensorQuadEncoder)
#pragma config(Motor,  port1,           claw,          tmotorVex393_HBridge, openLoop)
#pragma config(Motor,  port2,           topLiftLeft,   tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port3,           topLiftRight,  tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port4,           driveMotorsLeft, tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port5,           driveMotorsRight, tmotorVex393_MC29, openLoop, reversed)
#pragma config(Motor,  port6,           baseLiftLeft,  tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port7,           mobileLiftLeft, tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port8,           mobileLiftRight, tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port9,           baseLiftRight, tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port10,          claw2,         tmotorVex393_HBridge, openLoop, reversed)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

//NOTE: dgt3 and/or dgt4 do not work

#pragma systemFile

////GLOBAL VARIABLES////
//Poten Values For Lift -- Values increase as lift moves backwards
enum ForkliftPos {FORKLIFT_UP=1,FORKLIFT_DOWN=-1};
enum PotenValuesTop {BACK_TOP = 500, UPRIGHT_TOP = 2008, MATCHLOAD_TOP = 580, SCORE_TOP = 3000};
enum PotenValuesClaw {BACK_CLAW = 3700, MATCHLOAD_CLAW = 750};
enum PotenValuesBase {BACK_BASE = 4095, MATCHLOAD_BASE = 3200, HIGHEST_BASE =  2608}; //values increase as lift moves down
int topLiftPositions[12] = {3700,2600,2775,2600,2600,2600,2600,2600,2600,2600,2600,2600};
int baseLiftPositions[12] = {3600,3550,3400,3300,3100,3000,2900,3400,3300,3250,2695,2525};
int secondBaseLiftPositions[12] = {0,3450,3400,3300,3100,3000,2900,3400,3300,3250,2695,2525};

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

//for forkliftPos task/method
float forkliftPos = FORKLIFT_UP;

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
	motor[driveMotorsLeft] = power;
}

void setRightMotors(int power)
{
	motor[driveMotorsRight] = power;
}
void setAllDriveMotors(int power)
{
	setLeftMotors(power);
	setRightMotors(power);
}

void setTopLiftPower(int power)
{
	motor[topLiftLeft] = power;
	motor[topLiftRight] = power;
}


void setBaseLiftPower(int power)
{
	motor[baseLiftLeft] = power;
	motor[baseLiftRight] = power;
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
		deriv = (err-oldErr); //if error is increasing, apply more power (compensate for less momentum). else, apply more power
		integral = totalErr * 0.03;
		power = err + deriv + integral;
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
	if((desiredTop == BACK_TOP && SensorValue[topLiftPoten]<BACK_TOP) || reachedMobileGoal)
		ignore = true;
	int err = desiredTop - SensorValue[topLiftPoten];
	int power = 127;

	while(abs(err)>200 &&  !ignore) //adjust power of motors while error is outide of certain range, then set power to 0
	{
		err = desiredTop - SensorValue[topLiftPoten];
		power = (int) (-err*127/4095*kpTop);
		setTopLiftPower(power);
		//writeDebugStreamLine("Desired: %d, Poten: %d, Power: %d, Error: %d", desiredTop, SensorValue[topLiftPoten], power,err);
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
		power = (int) (err*127/2000*kpBase);
		setBaseLiftPower(power);
		//writeDebugStreamLine("Poten: %d, Power: %d, Error: %d", SensorValue[baseLiftPoten], power,err);
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

task setForkliftPosTask()
{
	clearTimer(T4);
	setForkliftPower(forkliftPos*127);
	while((SensorValue(forkliftButton) == 1 || forkliftPos==FORKLIFT_DOWN) && time1(T4)<1800){}
	setForkliftPower(0);
}

///////COMPLEX METHODS: a+bi///////
void setForkliftPos(int aForkPos)
{
	forkliftPos = aForkPos;
	startTask(setForkliftPosTask);
}
void driveStraight(int dest, int basePower, float rightMultiplier = 1) //uses correctStraight task (with gyro) to dive straight
{
	theta = SensorValue[gyro];
	SensorValue[leftQuad] = 0;
	SensorValue[rightQuad] = 0;
	int err = dest;
	int power = 127;
	startTask(correctStraight);
	//writeDebugStreamLine("err: %d, power: %d sdfdgdsgfgfsggffs",err,power);
	while(fabs(err)>20 && fabs(dest - (-1*SensorValue[leftQuad]))>20)
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

/////MORE COMPLEX TASKS - AUTOSTACKING///
task autoScoreTask()
{
	int topLiftStart = 1*topLiftPositions[conesStacked]; //poten units of base lift corresponing to top lift swing-around time, assuming poten increases towards score (increasing distance == increasing time alloted)
	//writeDebugStreamLine(" baseLiftPosDesired: %d", 	baseLiftPositions[conesStacked]);
	setBaseLiftPos(baseLiftPositions[conesStacked],SCORE_KP_BASE);
	//writeDebugStreamLine("must be less than this level: %d", baseLiftPositions[conesStacked] - topLiftStart - ERR_MARGIN);
	while(SensorValue[baseLiftPoten] > baseLiftPositions[conesStacked] + topLiftStart + ERR_MARGIN){wait1Msec(20);} //assuming poten decreases towards up
	wait1Msec(20);
	writeDebugStreamLine("first while loop in score");
	while(SensorValue[baseLiftPoten] > baseLiftPositions[conesStacked] + topLiftStart + ERR_MARGIN){wait1Msec(20);}
	writeDebugStreamLine("second while in score");
	setTopLiftPos(topLiftPositions[conesStacked],SCORE_KP_TOP);
	while(SensorValue[topLiftPoten] < topLiftPositions[conesStacked] - ERR_MARGIN){wait1Msec(20);}
	writeDebugStreamLine("third while in score");
	if(secondBaseLiftPositions[conesStacked] > 0 )
		setBaseLiftPos(secondBaseLiftPositions[conesStacked],SCORE_KP_BASE);
	conesStacked++;
	writeDebugStreamLine("last while in score");
}

task autoBackTask()
{
	userControlClaw = false;
	if(SensorValue[topLiftPoten]>UPRIGHT_TOP)
		setClawPower(127); //open claw
	setTopLiftPos(BACK_TOP,BACK_KP_TOP);
	while(SensorValue[topLiftPoten]>UPRIGHT_TOP){}
	setBaseLiftPos(BACK_BASE,BACK_KP_BASE,0);
	setClawPower(0); //close claw
}

task autoStackTask()
{
	int thisConeStack = conesStacked;
	startTask(autoScoreTask);
	while(SensorValue[topLiftPoten] < topLiftPositions[conesStacked] - ERR_MARGIN || SensorValue[baseLiftPoten] > baseLiftPositions[conesStacked] + ERR_MARGIN){wait1Msec(50);}
	writeDebugStreamLine("after 1st while in autoStack");
	//while(SensorValue[topLiftPoten] < topLiftPositions[conesStacked] - ERR_MARGIN || (secondBaseLiftPositions[conesStacked] > 0 && SensorValue[baseLiftPoten] > secondBaseLiftPositions[conesStacked] + ERR_MARGIN)){wait1Msec(50);}
	wait1Msec(200);
	writeDebugStreamLine("Starting autoBack task");
	startTask(autoBackTask);
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

void autoStack()
{
	startTask(autoStackTask);
}
