/* -----------------------------------------------------------------------
 * Paramics Programmer API    (paramics-support@quadstone.com)
 * Quadstone Ltd.             Tel: +44 131 220 4491
 * 16 Chester Street          Fax: +44 131 220 4492
 * Edinburgh, EH3 7RA, UK     WWW: http://www.paramics-online.com
 * ----------------------------------------------------------------------- */  

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include "programmer.h"
#include "Poco\Net\Socket.h"


Bool	drawUserPicks = PFALSE;

Bool getNode = PFALSE;
Bool getZone = PFALSE;
Bool getVeh = PFALSE;
Bool getLink = PFALSE;

NODE*	userNode = NULL;
ZONE*	userZone = NULL;
VEHICLE* userVeh = NULL;
LINK*	userLink = NULL;


// Called once after the network is loaded.
void qpx_NET_postOpen(void)
{
	// Add a customer layer in the Layer Selector in Modeller.
	qps_GUI_addViewLayer("User Picks");

	// Initialise flags.
	getNode = PFALSE;
	getZone = PFALSE;
	getVeh = PFALSE;
	getLink = PFALSE;

	userNode = NULL;
	userZone = NULL;
	userVeh = NULL;
	userLink = NULL;
}


// This is called when the User Picks checkbox in the Layer Selector
// has been toggled.
void qpx_GUI_layer(char *name, Bool on)
{
	if(strcmp(name,"User Picks")==0)	
	{
		drawUserPicks = on;
		// force a redraw
		qps_GUI_redraw();
	}
}


// Catch a key press from the main display.
void qpx_GUI_keyPress(int key, int ctrl, int shift, int left, int middle, int right)
{
	float x,y,z;	
	// check key combinations and return early if not approrrate
	// Key 3 && middle mouse = pick node
	// Key 4 && middle mouse = pick zone
	// Key 5 && middle mouse = pick vehicle
	// Key 6 && middle mouse = pick link

	// Set flags depending on pick.	
	getNode = PFALSE;
	getZone = PFALSE;
	getVeh = PFALSE;
	getLink = PFALSE;

	// What was the selection?
	if(key == 0x33 && middle) getNode = PTRUE; // 3 key
	if(key == 0x34 && middle) getZone = PTRUE; // 4 key
	if(key == 0x35 && middle) getVeh = PTRUE;  // 5 key
	if(key == 0x36 && middle) getLink = PTRUE; // 6 key

	// Get nearest object to the current mouse position.
	if(getNode)
	{
		qpg_POS_nearestNode(&userNode, &x, &y, &z);
	}
	else if(getZone)
	{
		qpg_POS_nearestZone(&userZone, &x, &y, &z);
	}
	else if(getVeh)
	{
		qpg_POS_nearestVehicle(&userVeh, &x, &y, &z);
	}
	else if(getLink)
	{		
		qpg_POS_nearestLink(&userLink, &x, &y, &z);	
	}

	// Force a redraw
	qps_GUI_redraw();
}

// At draw time, highlight the selected object based on our selections.
// NOTE that the User Picks checkbox should be set to on in the GUI.
void qpx_DRW_modelView(void)
{
	// Draw anything?
	if(!drawUserPicks) return;

	// Draw the correct selection
	if(getNode && userNode != NULL)
	{
		qps_DRW_colour(API_GREEN);
		qps_DRW_moveToNodeHome(userNode);
		qps_DRW_filledCircle(0,0,0,10);
	}
	else if(getZone && userZone != NULL)
	{
		qps_DRW_zoneFilled(userZone, API_BLUE, 50.0, 1);
	}
	else if(getVeh && userVeh != NULL)
	{
		qps_DRW_colour(API_YELLOW);
		qps_DRW_moveToVehicleHome(userVeh);
		qps_DRW_filledCircle(0,0,0,10);
	}
	else if(getLink && userLink != NULL)
	{
		qps_DRW_shadeLink(userLink, API_RED);
	}
}


// Cehicle arrives at destination, check if it's our selected vehicle and clear.
void qpx_VHC_arrive(VEHICLE* vehicle, LINK* link, ZONE* zone)
{
	if(vehicle == userVeh) userVeh = NULL;
}