// round.cpp

#include "stdafx.h"

#include "round.h"
#include <math.h>

void RoundTime(long& lLow, long& lHigh)
{
	long lRange = lHigh - lLow;
	
	if (lRange > 691200L)
	{
		// More than 192 hours - round to days
		lLow /= 86400L;
		lHigh /= 86400L;
		Round(lLow, lHigh);
		lLow *= 86400L;
		lHigh *= 86400L;
		return;
	}
	else if (lRange > 518400L) lRange = 691200L;	// 192 hours
	else if (lRange > 432000L) lRange = 518400L;	// 144 hours
	else if (lRange > 345600L) lRange = 432000L;	// 120 hours
	else if (lRange > 259200L) lRange = 345600L;	// 96 hours
	else if (lRange > 216000L) lRange = 259200L;	// 72 hours
	else if (lRange > 172800L) lRange = 216000L;	// 60 hours
	else if (lRange > 129600L) lRange = 172800L;	// 48 hours
	else if (lRange > 108000L) lRange = 129600L;	// 36 hours
	else if (lRange > 86400L) lRange = 108000L;		// 30 hours
	else if (lRange > 64800L) lRange = 86400L;		// 24 hours
	else if (lRange > 54000L) lRange = 64800L;		// 18 hours
	else if (lRange > 43200L) lRange = 54000L;		// 15 hours
	else if (lRange > 36000L) lRange = 43200L;		// 12 hours
	else if (lRange > 32400L) lRange = 36000L;		// 10 hours
	else if (lRange > 28800L) lRange = 32400L;		// 9 hours
	else if (lRange > 21600L) lRange = 28800L;		// 8 hours
	else if (lRange > 18000L) lRange = 21600L;		// 6 hours
	else if (lRange > 14400L) lRange = 18000L;		// 5 hours
	else if (lRange > 10800L) lRange = 14400L;		// 4 hours
	else if (lRange > 7200L) lRange = 10800L;		// 3 hours
	else if (lRange > 5400L) lRange = 7200L;		// 2 hours
	else if (lRange > 4320L) lRange = 5400L;		// 90 mins
	else if (lRange > 3600L) lRange = 4320L;		// 72 mins
	else if (lRange > 2880L) lRange = 3600L;		// 60 mins
	else if (lRange > 2160L) lRange = 2880L;		// 48 mins
	else if (lRange > 1800L) lRange = 2160L;		// 36 mins
	else if (lRange > 1440L) lRange = 1800L;		// 30 mins
	else if (lRange > 1080L) lRange = 1440L;		// 24 mins
	else if (lRange > 900L) lRange = 1080L;			// 18 mins
	else if (lRange > 720L) lRange = 900L;			// 15 mins
	else if (lRange > 600L) lRange = 720L;			// 12 mins
	else if (lRange > 540L) lRange = 600L;			// 10 mins
	else if (lRange > 480L) lRange = 540L;			// 9 mins
	else if (lRange > 360L) lRange = 480L;			// 8 mins
	else if (lRange > 300L) lRange = 360L;			// 6 mins
	else if (lRange > 240L) lRange = 300L;			// 5 mins
	else if (lRange > 180L) lRange = 240L;			// 4 mins
	else if (lRange > 120L) lRange = 180L;			// 3 mins
	else if (lRange > 90L) lRange = 120L;			// 2 mins
	else if (lRange > 60L) lRange = 90L;			// 90 secs
	else if (lRange > 30L) lRange = 60L;			// 60 secs
	else lRange = 30L;								// 30 secs
	
//	long lRoundUnit = lRange / 6L;

	long lRoundUnit;
	
	if (lRange > 64800L) lRoundUnit = 28800L;
	else if (lRange > 18000L) lRoundUnit = 3600L;
	else if (lRange > 2880L) lRoundUnit = 600L;
	else if (lRange > 300L) lRoundUnit = 60L;
	else if (lRange > 30L) lRoundUnit = 10L;
	else lRoundUnit = 5L;
	
	lLow = (lLow / lRoundUnit) * lRoundUnit;
	if (lHigh <= (lLow + lRange))
	{
		lHigh = lLow + lRange;
	}
	else
	{
		// Use next wider range
		RoundTime(lLow, lHigh);
	}
}


void Round(long& lLow, long& lHigh)
{
	long lRoundUnit;
	
	long lStartLow = lLow;
	long lStartHigh = lHigh;
	
	long lRange = lHigh - lLow;
	
	if (lRange == 0) lHigh++;
	
	if (lRange < 100L)
	{
		lRoundUnit = 10L;
	}
	else
	{
		// Get base rounding unit (1, 10, 100, ...)
		long lUnit = MakeLong((float)pow(10.0, (int)log10(lRange) - 1));
		// Get multiple of base unit (10 -> 99)
		long lMultiple = lRange / lUnit;
	
		if (lMultiple > 25) lRoundUnit = 10L;
		else if (lMultiple > 12L) lRoundUnit = 5L;
		else lRoundUnit = 2L;
	
		lRoundUnit = lRoundUnit * lUnit;	
	}
	
	// Round low value towards -ve infinity
	lLow = (lLow >= 0) ? (lLow / lRoundUnit) * lRoundUnit
					   : -(((-lLow - 1) / lRoundUnit) + 1) * lRoundUnit;

	// Round high value towards +ve infinity
	lHigh = (lHigh > 0) ? (((lHigh - 1) / lRoundUnit) + 1) * lRoundUnit
						: -(-lHigh / lRoundUnit) * lRoundUnit;
	
	// Make sure that re-rounding has no effect
	if ((lLow != lStartLow) || (lHigh != lStartHigh))
	{
		Round(lLow, lHigh);
	}
}


int TwoFigsToInt(char* acFigs)
{
	return((acFigs[0] - '0') * 10 + (acFigs[1] - '0'));
}


int MonthToInt(char* acMonth)
{
	switch (acMonth[0])
	{
		case 'J':
			switch (acMonth[1])
			{
				case 'a':
					return 1;			// Jan
				case 'u':
					switch (acMonth[2])
					{
						case 'n':
							return 6;	// Jun
						case 'l':
							return 7;	// Jul
					}
			}
			
		case 'F':
			return 2;					// Feb
			
		case 'M':
			switch (acMonth[2])
			{
				case 'r':
					return 3;			// Mar
				case 'y':
					return 5;			// May
			}
			
		case 'A':
			switch (acMonth[1])
			{
				case 'p':
					return 4;			// Apr
				case 'u':
					return 8;			// Aug
			}
			
		case 'S':
			return 9;					// Sep
			
		case 'O':
			return 10;					// Oct
		
		case 'N':
			return 11;					// Nov
			
		case 'D':
			return 12;					// Dec
	}
	
	return 1;
}


int MakeInt(float flValue)
{
	if (flValue > 0)
	{
		return (int)(flValue + 0.5F);
	}
	else
	{
		return (int)(flValue - 0.5F);
	}
}
	

long MakeLong(float flValue)
{
	if (flValue > 0)
	{
		return (long)(flValue + 0.5F);
	}
	else
	{
		return (long)(flValue - 0.5F);
	}
}


	