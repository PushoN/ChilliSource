/** 
 * File: TouchScreen.h
 * Date: 5 Oct 2010
 * Description: Concrete iOS3_x implementation of moflo::Input::ITouchScreen;
 */

/** 
 *
 * Author Stuart 
 * Version 1.0 - moFlo
 * Copyright ©2010 Tag Games Limited - All rights reserved 
 */

#ifndef _MOFLO_PLATFORM_IOS_TOUCHSCREEN_H_
#define _MOFLO_PLATFORM_IOS_TOUCHSCREEN_H_

#include <ChilliSource/Input/TouchScreen.h>
#include <UIKit/UIKit.h>

namespace moFlo
{
	namespace iOSPlatform
	{
		class CTouchScreen : public Input::ITouchScreen
		{
		public:
			CTouchScreen();
			~CTouchScreen();
			
			virtual bool IsA(Core::InterfaceIDType inInterfaceID) const;
			
			void OnTouchBegan(UITouch* inpNativeTouch);
			void OnTouchMoved(UITouch* inpNativeTouch);
			void OnTouchEnded(UITouch* inpNativeTouch);
            
		protected:
			
			typedef HASH_MAP<UITouch*, size_t> MapNativeTouchToID;
			MapNativeTouchToID mMapNativeTouchesToID;
            
            f32 mfScale;
		};
	}
}

extern "C" 
{
	void TouchBegan(UITouch* inTouch);
	void TouchMoved(UITouch* inTouch);
	void TouchEnded(UITouch* inTouch);
};
	
#endif