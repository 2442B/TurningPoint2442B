#pragma config(Sensor, in2,    secondBattery,  sensorAnalog)
#pragma config(Sensor, in3,    gyro,           sensorGyro)
#pragma config(Sensor, in7,    topLiftPoten,   sensorPotentiometer)
#pragma config(Sensor, in8,    baseLiftPoten,  sensorPotentiometer)
#pragma config(Sensor, dgtl1,  rightQuad,      sensorQuadEncoder)
#pragma config(Sensor, dgtl5,  forkliftButton, sensorDigitalIn)
#pragma config(Sensor, dgtl6,  greenLED,       sensorLEDtoVCC)
#pragma config(Sensor, dgtl7,  sideToggle,     sensorDigitalIn)
#pragma config(Sensor, dgtl8,  minorZoneToggle, sensorDigitalIn)
#pragma config(Sensor, dgtl9,  majorZoneToggle, sensorDigitalIn)
#pragma config(Sensor, dgtl10, leftQuad,       sensorQuadEncoder)
#pragma config(Motor,  port2,           rollers,       tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port3,           driveLeftFront, tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port4,           driveLeftBack, tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port5,           driveRightBack, tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port6,           driveRightFront, tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port7,           mobileLiftLeft, tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port8,           mobileLiftRight, tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port9,           baseLift,      tmotorVex393_MC29, openLoop)
#pragma config(Motor,  port10,          fourBar,       tmotorVex393_HBridge, openLoop, reversed)
//*!!Code automatically generated by 'ROBOTC' configuration wizard               !!*//

//NOTE: dgt3 and/or dgt4 do not work

#pragma platform(VEX2)

#pragma competitionControl(Competition)

#include "Vex_Competition_Includes.c"
#include "\motionPlanner.c"
#include "\InTheZoneLibrary.c"

//for auton task
string aMajorSide;
int aMinorSide = 1;
int aZone = 10;

void waitForRelease()
{
	while(nLCDButtons != 0){wait1Msec(5);}
}

void pre_auton()
{
	bLCDBacklight = true;
	displayLCDCenteredString(0,"Initializing");
	displayLCDCenteredString(1,"Gyro");
	writeDebugStreamLine("begin gyro init");
	SensorType[in4] = sensorNone;
	wait1Msec(1000);
	SensorType[in4] = sensorGyro;
	wait1Msec(2000);
	SensorScale[in4] = 133;
	writeDebugStreamLine("finished gyro init %d", SensorScale[in4]);

	string mainBattery;
	string powerExpander;
	string selection[6] = {"20-pt zone [L]", "20-pt zone [R]", "10-pt zone [L]", "10-pt zone [R]", "5-pt zone [L]", "5-pt zone [R]"};
	int selectionSide[6] = {1,-1,1,-1,1,-1};
	int selectionZone[6] = {20,20,10,10,5,5};
	bool showBattery = true;
	int autonSelect = 0;
	int count = 0;
	int sideSelect = 1;

	while(bIfiRobotDisabled)
	{
		if(nLCDButtons == 2)//center button
		{
			waitForRelease();
			showBattery = !showBattery;
		}
		if(!showBattery)
		{
			if(nLCDButtons == 1)//left button
			{
				count--;
			}
			else if(nLCDButtons == 4)//right button
			{
				waitForRelease();
				count++;
			}
		}


		if(count>5){count=count%5-1;}
		else if(count<0){count = count+5 + 1;}

		clearLCDLine(0);
		if(showBattery)
		{
			sprintf(mainBattery, "Main: %f", nImmediateBatteryLevel/1000.0);
			sprintf(powerExpander, "2nd Battery: %f", SensorValue[secondBattery]/155.0);
			displayLCDCenteredString(0,mainBattery);
			displayLCDCenteredString(1,powerExpander);
			//writeDebugStreamLine("powerExpander: %f", powerExpander);
		}
		else
		{
			displayLCDCenteredString(0,selection[count]);
			clearLCDLine(1);
			aMinorSide = selectionSide[count];
			aZone = selectionZone[count];
			waitForRelease();
		}

		wait1Msec(25);
	}

	//aMajorSide = "blue";

	//if(SensorValue[sideToggle] == 1) //if empty (1), then side is left (1), else side is right (-1)
	//	aMinorSide = 1;
	//else
	//	aMinorSide = -1;

	//if(SensorValue[majorZoneToggle] == 0) //if jumper is in (0), zone is 20
	//	aZone = 20;
	//else if(SensorValue[minorZoneToggle] == 1) //if empty (and majorZone is empty), zone is 10, else 5
	//	aZone = 10;
	//else
	//	aZone = 5;
	////white line -- -1315

	//	writeDebugStreamLine("Zone in preAuton: %d",aZone);

	//SensorValue[greenLED] = 1;

}

void runBasicCompAutonFake()
{
	setupMotorProfiles();

	profileGoTo(port3,1700);
	profileGoTo(port6,1700);

	//writeDebugStreamLine("done with profileGoTo");

	//profileGoTo(port3,1000);
	//profileGoTo(port6,1000);

}

void runBasicCompAuton(int minorSide, int zone)
{
	//minorSide: 1 = left, -1 = right, majorSide parameter not used yet
	clearTimer(T1);
	reachedMobileGoal = false; //will act as hard stop for lifting cone �?? when reachedMobileGoal is true, the lift will immediately drop

	//Go to mobile goal � Drop mobile base lift, lift cone, and drive straight
	motor[rollers] = -20;
	setBaseLiftPos(550, 10, -15);
	setForkliftPower(-80);
	driveStraightDistance(2000); //drive to mobile goal
	setForkliftPower(0);

	//pick up goal
	reachedMobileGoal = true; //force cone lift to drop
	setForkliftPos(FORKLIFT_UP);
	wait1Msec(1200);
	setBaseLiftPos(800, 10);
	turnToPos(70, true, 500);
	setClawPower(127);
	//driveStraight(70,127); //for second
	wait1Msec(500);
	setForkliftPower(0);
	setClawPower(0);

	//pick up second cone
	/*
	setClawPower(-80);
	wait1Msec(200);
	setClawPower(0);
	wait1Msec(200);
	setClawPower(127);
	holdTopLiftPos(basicTopPositions[0],1);
	setBaseLiftPos(1200, 100); //full power down
	while(SensorValue[topLiftPoten]>basicTopPositions[0]+100){wait1Msec(50);}
	wait1Msec(300);
	setTopLiftPos(basicTopPositions[2],basicTopKp[2]);
	setClawPower(0);
	*/
	if(zone==20)
	{
		turnToPos(110*minorSide, true, 700);
		/*
		if(SensorValue[gyro]>110*minorSide)
		{
		setLeftMotors(30);
		setRightMotors(-30);
		while(SensorValue[gyro]>minorSide*115){wait1Msec(20);}
		}
		else if(SensorValue[gyro]<105*minorSide)
		{
		setLeftMotors(-30);
		setRightMotors(30);
		while(SensorValue[gyro]<110*minorSide){wait1Msec(20);}
		}
		*/
		setBaseLiftPos(950, 10);
		driveStraight(-1800,127);
	}
	else if(zone == 10)
	{
		//drive back
		turnToPos(0);
		setBaseLiftPos(950, 10);
		driveStraight(-1200,127); //drive back -1000
	}
	else
	{
		turnToPos(0);
		setBaseLiftPos(950, 10);
		driveStraight(-750,127); //drive back -1000
	}
	setClawPower(127);
	wait1Msec(500);
	setBaseLiftPos(500, 10);

	//Score goal
	if(zone == 20)
	{
		turnToPos(1337*minorSide);
		basicSlewControlDrive(127);
		wait1Msec(750);
		setForkliftPos(FORKLIFT_DOWN);
		wait1Msec(500);
		//driveStraight(220,127);
	}
	else if(zone == 10)
	{
		turnToPos(-1800*minorSide);
		setClawPower(0);

		//swing turn
		if(minorSide == 1){
			setLeftMotors(127);
			setRightMotors(0);
		}
		else
		{
			setLeftMotors(0);
			setRightMotors(127);
		}
		while(fabs(SensorValue[gyro]) < 2200) {}
		setAllDriveMotors(0);
		setForkliftPos(FORKLIFT_DOWN);
		wait1Msec(1800);
	}
	else if(zone == 5)
	{
		//just turn around and drive straight
		turnToPos(-1800*minorSide);
		setClawPower(0);
		setLeftMotors(127);
		setRightMotors(0);
		while(SensorValue[gyro] < -2145 * minorSide) {}
		setAllDriveMotors(0);
		setForkliftPos(FORKLIFT_DOWN);
	}

	setClawPower(0);
	wait1Msec(200);
	driveStraight(-500,127,1);
	writeDebugStreamLine("Time: %d", time1(T1));
}

task runEndAuton()
{
	setForkliftPos(FORKLIFT_DOWN);
	wait1Msec(1100);
	//setClawPower(0);
	driveStraight(-500,127,1);
	setTopLiftPower(0);
	writeDebugStreamLine("Time: %d", time1(T1));
}

task autonomous()
{
	//majorSide = "blue";
	//minorSide = -1; //1 = left, -1 = right
	//zone = 5; //choose 5, 10, or 20
	//clearTimer(T3);
	writeDebugStreamLine("the zone %d",aZone);
	writeDebugStreamLine("the side %d",aMinorSide);
	runBasicCompAuton(aMinorSide,aZone);
	//while(time1(T3)<14500){wait1Msec(20);}
	//stopTask(runBasicCompAuton);
	//startTask(runEndAuton);

	//runProgSkills();
	//basicSlewControlDrive(127);
	//driveStraight(1000,90);
}

task usercontrol()
{
	stopTask(runEndAuton);
	bool coneUpPressed = false;
	bool coneDownPressed = false;
	bool coneZeroPressed = false;
	bool autoStackPressed = false;
	bool runIntake = false;
	//bool rollerPressed = false;
	//bool rollerMovingOpen = false;
	//bool rollerMovingClosed = false;
	bool topLiftPressed = false;
	int topLiftTargetLoc = 0; // 2 = up; 1 = flat; 0 = down

SensorValue[rightQuad]=0;
SensorValue[leftQuad] = 0;
	while(true)
	{
		if(vexRT[Btn7L]==1)
		{
			runBasicCompAutonFake()
			wait1Msec(7000);
		}

		if(vexRT[Btn7R]==1)
		{
			int side = 1;
			int zone = 10;
			runBasicCompAuton(side,zone);
		}

		//Buttons and Joysticks
		int  rightJoy = vexRT[Ch2];
		int  leftJoy = vexRT[Ch3];
		word rightTriggerUp = vexRT[Btn6U]; //for score top lift
		word rightTriggerDown = vexRT[Btn6D]; //for back top lift
		word leftTriggerUp = vexRT[Btn5U]; //for pincer close
		word leftTriggerDown = vexRT[Btn5D]; //for pincer open
		word btnEightUp = vexRT[Btn8U]; //for up base lift
		word btnEightDown = vexRT[Btn8D]; //for down base lift
		word btnSevenUp = vexRT[Btn7U]; //forklift up
		word btnSevenDown = vexRT[Btn7D]; //forklift down
		word btnEightLeft = vexRT[Btn8L]; //auto score
		word btnEightRight = vexRT[Btn8R]; //auto back
		word btnSevenLeft = vexRT[Btn7L]; //auton testing
		word btnSevenRight = vexRT[Btn7R]; //move to matchloads
		word secondBtnSevenUp = vexRT[Btn7UXmtr2]; //+1 to cone count
		word secondBtnSevenDown = vexRT[Btn7DXmtr2]; //-1 to cone count
		word secondBtnSevenLeft = vexRT[Btn7LXmtr2]; //zero cone count

		//Drive Motors
		if(fabs(rightJoy) >= 15)
			setRightMotors(rightJoy);
		else
			setRightMotors(0);

		if(fabs(leftJoy) >= 15)
			setLeftMotors(leftJoy);
		else
			setLeftMotors(0);


		//TOP LIFT
		if(rightTriggerUp == 1)
		{

			if(!topLiftPressed)
			{
				//increase target by 1 up to max position
				topLiftTargetLoc++;
				writeDebugStreamLine("unfiltered topLiftTargetLoc: %f", topLiftTargetLoc);
				if(topLiftTargetLoc > 1){topLiftTargetLoc = 1;}
				writeDebugStreamLine("only one topLiftTargetLoc: %f", topLiftTargetLoc);
				//bring to target pos
				holdTopLiftPos(basicTopPositions[topLiftTargetLoc],0.3);
			}

			topLiftPressed = true;
		}
		else if(rightTriggerDown == 1)
		{
			//setTopLiftPower(-80);

			if(!topLiftPressed)
			{
				//decrease target by 1 up to max position
				topLiftTargetLoc--;
				if(topLiftTargetLoc < 0){topLiftTargetLoc = 0;}
				writeDebugStreamLine("only one topLiftTargetLoc: %f", topLiftTargetLoc);
				//bring to target pos
				holdTopLiftPos(basicTopPositions[topLiftTargetLoc],0.3);
			}

			topLiftPressed = true;
		}

		else
		{
			topLiftPressed = false;
			//setTopLiftPower(0);
		}


		//BASE LIFT
		if(btnEightUp == 1)
		{
			if(userControlBase)
				setBaseLiftPower(-127);
			else
				userControlBase = true;
		}
		else if(btnEightDown == 1)
		{
			if(userControlBase)
				setBaseLiftPower(80);
			else
				userControlBase = true;
		}
		else
		{
			if(userControlBase)
				setBaseLiftPower(0);
		}

		//Mobile Goal Base Lifters
		if(btnSevenUp == 1)
		{
			if(SensorValue(forkliftButton) == 1)
				setForkliftPower(127);
			else
				setForkliftPower(0);
		}
		else if(btnSevenDown == 1)
			setForkliftPower(-127);
		else
			setForkliftPower(0);


		//roller
		if(leftTriggerUp == 1) {
			motor[rollers] = 127;
			runIntake = false;
		}
		else if(leftTriggerDown == 1) {
			motor[rollers] = -127;
			runIntake = true;
		}
		else {
			if (runIntake)
				motor[rollers] = -20;
			else
				motor[rollers] = 0;
		}
		/*
		if(leftTriggerDown == 1)
		{
		if(userControlClaw) //check if used in automation
		{
		if(!rollerPressed) //make button into toggle
		{
		if(!rollerMovingClosed)//if not moving, close roller. if moving, stop roller
		setClawPower(-80);
		else
		setClawPower(0);

		rollerMovingClosed = !rollerMovingClosed;
		rollerMovingOpen = false;
		}
		rollerPressed = true;
		}
		else
		userControlClaw = true;
		}
		else if(leftTriggerUp == 1)
		{
		if(userControlClaw)
		{
		if(!rollerPressed)
		{
		if(!rollerMovingOpen)//if not moving, open roller. if moving, stop roller
		setClawPower(80);
		else
		setClawPower(0);

		rollerMovingOpen = !rollerMovingOpen;
		rollerMovingClosed = false;
		}
		rollerPressed = true;
		}
		else
		userControlClaw = true;
		}
		else
		{
		if(userControlClaw)
		rollerPressed = false;
		}
		*/
		if(btnEightLeft == 1)
		{
			holdBaseLiftPos(BACK_BASE);
		}
		if(btnEightRight == 1)
		{
			holdBaseLiftPos(MATCHLOAD_BASE);
		}
		//AUTO METHODS
		/*
		if(btnEightLeft == 1){autoBack();}
		else if(btnEightRight == 1 && !autoStackPressed) //if button is now pressed, update cones and update bool to reflect button pressed
		{
		autoStackPressed = true;
		autoStack();
		writeDebugStreamLine("Cones Stacked: %d", conesStacked);
		}
		else if(btnEightRight == 0 && autoStackPressed) //if button is no longer pressed, update bool to reflect lack of press
		{
		autoStackPressed = false;
		}

		//if(btnSevenRight == 1)
		//{
		//	setBaseLiftPos(MATCHLOAD_BASE,MATCHLOAD_KP_BASE);
		//	setTopLiftPos(MATCHLOAD_TOP,MATCHLOAD_KP_TOP);
		//}

		if(btnEightLeft == 1){autoBack();}

		//else if(btnEightRight == 1 && !autoStackPressed) //if button is now pressed, update cones and update bool to reflect button pressed
		//{
		//autoStackPressed = true;
		//autoStack();
		//writeDebugStreamLine("Cones Stacked: %d", conesStacked);
		//}
		//else if(btnEightRight == 0 && autoStackPressed) //if button is no longer pressed, update bool to reflect lack of press
		//{
		//autoStackPressed = false;
		//}

		//cone count
		if(secondBtnSevenUp == 1 && !coneUpPressed) //if button is now pressed, update cones and update bool to reflect button pressed
		{
		conesStacked++;
		coneUpPressed = true;
		}
		else if(secondBtnSevenUp == 0 && coneUpPressed) //if button is no longer pressed, update bool to reflect lack of press
		{
		coneUpPressed = false;
		}

		if(secondBtnSevenDown == 1 && !coneDownPressed) //if button is now pressed, update cones and update bool to reflect button pressed
		{
		conesStacked--;
		coneDownPressed = true;
		}
		else if(secondBtnSevenDown == 0 && coneDownPressed) //if button is no longer pressed, update bool to reflect lack of press
		{
		coneDownPressed = false;
		}

		if(secondBtnSevenLeft == 1 && !coneZeroPressed) //if button is now pressed, update cones and update bool to reflect button pressed
		{
		conesStacked = 0;
		coneZeroPressed = true;
		}
		else if(secondBtnSevenLeft == 0 && coneZeroPressed) //if button is no longer pressed, update bool to reflect lack of press
		{
		coneZeroPressed = false;
		}
		*/
	}
}

/*
//PROGRAMMING SKILLS
void runProgSkills()
{
	clearTimer(T1);
	//run auton to score in 20Z
	string blank = "";
	reachedMobileGoal = false; //will act as hard stop for lifting cone �?? when reachedMobileGoal is true, the lift will immediately drop

	//Go to mobile goal � Drop mobile base lift, lift cone, and drive straight
	setBaseLiftPos(550, 10, -15);
	setForkliftPower(-90);
	//basicSlewControlDrive(127);
	driveStraight(1450,90); //drive to mobile goal
	setForkliftPower(0);

	//pick up goal
	reachedMobileGoal = true; //force cone lift to drop
	setForkliftPos(FORKLIFT_UP);
	wait1Msec(1500);
	turnToPos(-100, true, 2000);
	wait1Msec(200);
	setBaseLiftPos(800, 10);
	setForkliftPower(0);

	basicSlewControlDrive(-127);
	driveStraight(-1975, 90); //Back up
	turnToPos(-1350, true, 2500);
	if(SensorValue[gyro]>-1350)
	{
		setLeftMotors(30);
		setRightMotors(-30);
		while(SensorValue[gyro]>-1340){wait1Msec(20);}
		setAllDriveMotors(0);
	}

	setClawPower(127); //Drop cone
	wait1Msec(500);
	//setBaseLiftPos(500, 10); //Raise baseLift
	setClawPower(0);

	//score into 20
	basicSlewControlDrive(127);
	wait1Msec(750);
	setForkliftPos(FORKLIFT_DOWN);
	wait1Msec(500);

	//back out
	setBaseLiftPos(750, 10);
	driveStraight(-500, 127);
	setForkliftPos(FORKLIFT_UP);

	//reset against bar
	wait1Msec(500);
	turnToPos(425,false,2000);
	//basicSlewControlDrive(-127);
	driveStraight(-10000,40,1,2000);//distance is arbitrarily large so that time is a limiting factor
	wait1Msec(500);

	//reset along wall for first time
	SensorValue[gyro] = 0;
	driveStraight(150,90);
	setAllDriveMotors(0);
	wait1Msec(200);
	turnToPos(-800);
	setLeftMotors(25);
	while(SensorValue[gyro] > -890){wait1Msec(40);}
	setLeftMotors(0);
	wait1Msec(500);
	setAllDriveMotors(-127);
	wait1Msec(3000); //Back into wall

	//move to second mogo
	setAllDriveMotors(0);
	setBaseLiftPower(-90);
	while(SensorValue[baseLiftPoten]>300){wait1Msec(20);}
	setBaseLiftPower(-10);
	wait1Msec(100);
	setForkliftPos(FORKLIFT_DOWN);
	driveStraight(550,80);
	turnToPos(0);
	if(SensorValue[gyro]>15)
	{
		setLeftMotors(30);
		setRightMotors(-30);
		while(SensorValue[gyro]>5){wait1Msec(20);}
	}
	else if(SensorValue[gyro]<-15)
	{
		setLeftMotors(-30);
		setRightMotors(30);
		while(SensorValue[gyro]<-5){wait1Msec(20);}
	}
	setAllDriveMotors(0);
	wait1Msec(200);
	setForkliftPos(-60);
	driveStraight(550,90);
	setForkliftPos(FORKLIFT_UP);
	wait1Msec(1500);

	//Holding second mobile base
	//turn and score
	driveStraight(-350, 90);
	wait1Msec(200);
	turnToPos(-1675, true);
	wait1Msec(200);
	driveStraight(330,90);
	setLeftMotors(30);
	while(SensorValue[gyro]>-1750){wait1Msec(40);}
	setAllDriveMotors(0);
	wait1Msec(500);
	setForkliftPos(FORKLIFT_DOWN);

	wait1Msec(1200);
	//back out of 2nd mogo
	driveStraight(-200, 80);

	//correct against bar after 2nd mogo
	setForkliftPos(FORKLIFT_UP);
	wait1Msec(1800);
	turnToPos(0);
	//basicSlewControlDrive(-80);
	driveStraight(-10000,40,1,1700);

	//turn and drive to 3rd mogo
	SensorValue[gyro] = 0;
	wait1Msec(500);
	setRightMotors(30);
	while(SensorValue[gyro]<20){wait1Msec(20);}
	setAllDriveMotors(0);
	wait1Msec(500);
	setForkliftPower(-60);
	driveStraight(2000,80);

	//Holding 3rd mogo

	//setForkliftPos(FORKLIFT_UP);

	driveStraight(1350, 127); //previously 1150
	driveStraight(-800, 127);
	/*
	turnToPos(-900);
	if(SensorValue[gyro]<-900)
	{
		while(SensorValue[gyro]<-900)
		{
			setLeftMotors(-30);
			setRightMotors(30);
		}
		setAllDriveMotors(0);
	}
	else
	{
		while(SensorValue[gyro]>-900)
		{
			setLeftMotors(30);
			setRightMotors(-30);
		}
		setAllDriveMotors(0);
	}
	wait1Msec(100);
	driveStraight(250, 90);
	wait1Msec(100);

	turnToPos(0);
	wait1Msec(200);

	basicSlewControlDrive(127)
	wait1Msec(750);
	setForkliftPos(FORKLIFT_DOWN);
	wait1Msec(500);

	driveStraight(-1000,127);

	//--
	writeDebugStreamLine("PROG SKILLS TIME: %f", time1(T1));

	//	turnToPos(-425, false, 2500); //Around along white line
	//	driveStraight(-10000, 127,1,1000); //Back up
	//	SensorValue[gyro] = 0;
	//turnToPos(900, false, 2500); //Turn perpendicular to bar

	//CALIBRATE GYRO

	/*

	--Second Mobile Goal--

	back
	reset gyro -- Angles are now different
	turn clockwise
	forward
	pick up
	backwards
	turn counterclockwise 180
	mobile base down

	--Third Mobile Goal--

	back
	clockwise 180*
	forward
	wait, then base lift up
	base lift down
	turn counterclockwise 135*
	forward
	base lift up
	back
	clockwise 45*
	back clockwise 90*
	forward
	mobile base down
}
	*/
