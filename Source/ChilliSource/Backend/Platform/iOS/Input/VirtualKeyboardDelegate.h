//
//  VirtualKeyboardDelegate.h
//  iOSTemplate
//
//  Created by Scott Downie on 18/07/2011.
//  Copyright 2011 Tag Games. All rights reserved.
//

#ifndef _MO_FLO_PLATFORM_IOS_INPUT_VIRTUAL_KEYBOARD_DELEGATE_H_
#define _MO_FLO_PLATFORM_IOS_INPUT_VIRTUAL_KEYBOARD_DELEGATE_H_

#include <UIKit/UIKit.h>
#include <ChilliSource/Platform/iOS/Input/VirtualKeyboard.h>

using namespace moFlo::iOSPlatform;

@interface VirtualKeyboardDelegate : NSObject<UITextFieldDelegate>
{
    CVirtualKeyboard* mpKeyboard;
}

-(VirtualKeyboardDelegate*) initWithKeyboard:(CVirtualKeyboard*) inpKeyboard;

@end

#endif