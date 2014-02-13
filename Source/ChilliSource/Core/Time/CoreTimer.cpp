/*
 * File: CoreTimer.cpp
 * Date: 09/11/2010 2010 
 * Description: 
 */

/*
 * Author: Scott Downie
 * Version: v 1.0
 * Copyright ©2010 Tag Games Limited - All rights reserved 
 */

#include <ChilliSource/Core/CoreTimer.h>

namespace moFlo
{
	namespace Core
	{
        CEvent1<TimeEventDelegate> CCoreTimer::mOnTimerUpdateEvent;
        
		//----------------------------------------------------
		/// Get Timer Update Event
		///
		/// @return The event
		//----------------------------------------------------
		IEvent<TimeEventDelegate>& CCoreTimer::GetTimerUpdateEvent()
		{
			return mOnTimerUpdateEvent;
		}
        //----------------------------------------------------
        /// Update
        ///
        /// @param Time since last frame
        //----------------------------------------------------
		void CCoreTimer::Update(f32 infDT)
		{
			mOnTimerUpdateEvent.Invoke(infDT);
		}
	}
}
