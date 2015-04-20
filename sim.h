//===========================================================================
// FILE: sim.h
//===========================================================================
// Header file for simulation code
// Author: Barnaby Stewart (P14167376)
//---------------------------------------------------------------------------
#ifndef SIM_H
#define SIM_H



typedef struct 
{
	int timeLimit;
	int brwrQty;
	int brwrDelay;
	int brwrOffset;
	int brwrRqstSize;
	int lbrnDelay;
	int lbrnRqstSize;
	int lbryBookRange;
	int lbryNumBooks;
} config_t;

extern config_t config;


#endif//SIM_H