/** 
 * File: Touchscreen.h
 * Date: 5 Oct 2010
 * Description: Interface for a TouchScreen input device
 */

/** 
 *
 * Author Stuart 
 * Version 1.0 - moFlo
 * Copyright ©2010 Tag Games Limited - All rights reserved 
 */

#ifndef _MOFLO_INPUT_TOUCHSCREEN_H_
#define _MOFLO_INPUT_TOUCHSCREEN_H_

#include <ChilliSource/Input/InputDevice.h>
#include <ChilliSource/Core/FastDelegate.h>
#include <ChilliSource/Core/Math/Vector2.h>
#include <ChilliSource/Core/Math/Matrix4x4.h>
#include <ChilliSource/Core/GenericEvent.h>

#include <ChilliSource/Core/boost/thread/condition.hpp>

namespace moFlo
{
	namespace Input
	{
        enum TouchInputType
        {
            TOUCH_BEGAN,
            TOUCH_MOVED,
            TOUCH_ENDED,
        };
		struct TouchInfo
		{
			bool operator ==(const TouchInfo & inOther);
			
			Core::CVector2 vLocation; //Where the touch is now in screenspace
			Core::CVector2 vPreviousLocation; //Where it was last update cycle in screenspace
			f64 TimeStamp;
            u32 ID; //Unique identifier for this touch
            TouchInputType eType;
		};
		
		//Delegates
		
		typedef fastdelegate::FastDelegate1<const TouchInfo &> TouchEventDelegate;
		
		class ITouchScreen : public IInputDevice
		{
		public:
			ITouchScreen();
			typedef DYNAMIC_ARRAY<TouchInfo> TouchList;
			typedef TouchList::iterator TouchListItr;

			virtual ~ITouchScreen();
			
			DECLARE_NAMED_INTERFACE(ITouchScreen);
			
			IEvent<TouchEventDelegate> & GetTouchBeganEvent();
			IEvent<TouchEventDelegate> & GetTouchMovedEvent();
			IEvent<TouchEventDelegate> & GetTouchEndEvent();
			
            void Enable();
            void Disable();
            
			void SetTouchTransformMatrix(const Core::CMatrix4x4 & inTouchTransformMat);
			void SetCurrentAppTime(f64 inffTimeStamp);
			void SetScreenHeight(u32 inudwScreenHeight);
            
            //-----------------------------------------------------------
            /// Flush Buffered Input
            ///
            /// Have the input elements notify listeners of each
            /// buffered value then clear the buffered input
            //-----------------------------------------------------------
            void FlushBufferedInput();

			u32 StartTouch(const Core::CVector2 & invTouchLocation, f64 inTimeStamp);
			void MoveTouch(u32 inID, const Core::CVector2 & invNewTouchLocation, f64 inTimeStamp);
			void EndTouch(u32 inID, f64 inTimeStamp);

			f64 GetLastTimeStamp() const{return mffLastTimeStamp;}
			TouchInfo& GetTouch(u32 inudwID) { return maActiveTouches[inudwID]; }
			TouchList& GetTouchList() { return maActiveTouches; }
		protected:
            //-----------------------------------------------------------
            /// Add Active Touch
            ///
            /// Adds a new touch to the active touch list.
            ///
            /// @param The touch info.
            //-----------------------------------------------------------
            void AddActiveTouch(const TouchInfo& inTouchInfo);
            //-----------------------------------------------------------
            /// Update Active Touch
            ///
            /// Updates an existing touch in the active touch list.
            ///
            /// @param The touch info.
            //-----------------------------------------------------------
            void UpdateActiveTouch(const TouchInfo& inTouchInfo);
            //-----------------------------------------------------------
            /// Remove Active Touch
            ///
            /// Removes an existing touch from the active list.
            ///
            /// @param The touch info.
            //-----------------------------------------------------------
            void RemoveActiveTouch(const TouchInfo& inTouchInfo);
            
			CEvent1<TouchEventDelegate> mTouchBeganEvent;
			CEvent1<TouchEventDelegate> mTouchMovedEvent;			
			CEvent1<TouchEventDelegate> mTouchEndedEvent;
			
            
			TouchList mOpenTouches;
            TouchList maActiveTouches;
            TouchList mBufferedTouches;
			
			u32 mLastTouchIndex;
			f64 mffLastTimeStamp;
			
			Core::CMatrix4x4 mTouchTransformMatrix;
			
			u32 mudwOrientedScreenHeight;
			boost::mutex mMutex;
            
            bool mbIsEnabled;
		};
		
	}
}

#endif