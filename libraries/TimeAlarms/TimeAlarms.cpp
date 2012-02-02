/*
  TimeAlarms.cpp - Arduino Time alarms for use with Time library   
  Copyright (c) 208-2011 Michael Margolis. 
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
 */
 
 /*
  2 July 2011 - replaced alarm types implied from alarm value with enums to make trigger logic more robust
              - this fixes bug in repeating weekly alarms - thanks to Vincent Valdy and draythomp for testing
*/

extern "C" {
#include <string.h> // for memset
}

#if defined(ARDUINO) && ARDUINO >= 100
  #include "Arduino.h"
#else
  #include "WProgram.h"
#endif

#include "TimeAlarms.h"
#include "Time.h"

#define IS_ONESHOT  true   // constants used in arguments to create method
#define IS_REPEAT   false 


//**************************************************************
//* Alarm Class Constructor

AlarmClass::AlarmClass()
{
  Mode.isEnabled = Mode.isOneShot = 0;
  Mode.alarmType = dtNotAllocated;
  value = nextTrigger = 0;
  onTickHandler = NULL;  // prevent a callback until this pointer is explicitly set 
}

//**************************************************************
//* Private Methods

 
void AlarmClass::updateNextTrigger()
{  
  if( (value != 0) && Mode.isEnabled )
  {
    time_t time = now();
    if( dtIsAlarm(Mode.alarmType) && nextTrigger <= time )   // update alarm if next trigger is not yet in the future
    {      
      if(Mode.alarmType == dtExplicitAlarm ) // is the value a specific date and time in the future
      {
        nextTrigger = value;  // yes, trigger on this value   
      }
      else if(Mode.alarmType == dtDailyAlarm)  //if this is a daily alarm
      {
        if( value + previousMidnight(now()) <= time)
        {
          nextTrigger = value + nextMidnight(time); // if time has passed then set for tomorrow
        }
        else
        {
          nextTrigger = value + previousMidnight(time);  // set the date to today and add the time given in value   
        }
      }
      else if(Mode.alarmType == dtWeeklyAlarm)  // if this is a weekly alarm
      {
        if( (value + previousSunday(now())) <= time)
        {
          nextTrigger = value + nextSunday(time); // if day has passed then set for the next week.
        }
        else
        {
          nextTrigger = value + previousSunday(time);  // set the date to this week today and add the time given in value 
        } 
      }
      else  // its not a recognized alarm type - this should not happen 
      {
        Mode.isEnabled = 0;  // Disable the alarm
      }	  
    }
    if( Mode.alarmType == dtTimer)
    {
      // its a timer
      nextTrigger = time + value;  // add the value to previous time (this ensures delay always at least Value seconds)
    }
  }
  else
  {
    Mode.isEnabled = 0;  // Disable if the value is 0
  }
}

//**************************************************************
//* Time Alarms Public Methods

TimeAlarmsClass::TimeAlarmsClass()
{
  isServicing = false;
  for(uint8_t id = 0; id < dtNBR_ALARMS; id++)
     free(id);   // ensure  all Alarms are cleared and available for allocation  
}

// this method creates a trigger at the given absolute time_t
// it replaces the call to alarmOnce with values greater than a week   
AlarmID_t TimeAlarmsClass::triggerOnce(time_t value, OnTick_t onTickHandler){   // trigger once at the given time_t
     if( value > 0)
        return create( value, onTickHandler, IS_ONESHOT, dtExplicitAlarm );
     else
        return dtINVALID_ALARM_ID; // dont't allocate if the time is greater than one day 	  
}

// this method will now return an error if the value is greater than one day - use DOW methods for weekly alarms   
AlarmID_t TimeAlarmsClass::alarmOnce(time_t value, OnTick_t onTickHandler){   // trigger once at the given time of day
     if( value <= SECS_PER_DAY)
        return create( value, onTickHandler, IS_ONESHOT, dtDailyAlarm );
     else
        return dtINVALID_ALARM_ID; // dont't allocate if the time is greater than one day 	  
}

AlarmID_t TimeAlarmsClass::alarmOnce(const int H,  const int M,  const int S,OnTick_t onTickHandler){   // as above with HMS arguments
   return create( AlarmHMS(H,M,S), onTickHandler, IS_ONESHOT, dtDailyAlarm );
}

AlarmID_t TimeAlarmsClass::alarmOnce(const timeDayOfWeek_t DOW, const int H,  const int M,  const int S, OnTick_t onTickHandler){  // as above, with day of week 
   return create( (DOW-1) * SECS_PER_DAY + AlarmHMS(H,M,S), onTickHandler, IS_ONESHOT, dtWeeklyAlarm );   
}
   
// this method will now return an error if the value is greater than one day - use DOW methods for weekly alarms   
AlarmID_t TimeAlarmsClass::alarmRepeat(time_t value, OnTick_t onTickHandler){ // trigger daily at the given time
    if( value <= SECS_PER_DAY)
       return create( value, onTickHandler, IS_REPEAT, dtDailyAlarm );
    else
       return dtINVALID_ALARM_ID; // dont't allocate if the time is greater than one day 	  
    void TimeAlarmsClass::free(AlarmID_t ID)
    {
      if(isAllocated(ID))
      {
        Alarm[ID].Mode.isEnabled = false;
    	Alarm[ID].Mode.alarmType = dtNotAllocated;
        Alarm[ID].onTickHandler = 0;
    	Alarm[ID].value = 0;
    	Alarm[ID].nextTrigger = 0;   	
      }
    }
    