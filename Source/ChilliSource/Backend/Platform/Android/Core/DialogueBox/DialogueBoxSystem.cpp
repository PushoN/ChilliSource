//
//  DialogueBoxSystem.cpp
//  Chilli Source
//
//  Created by Ian Copland on 04/03/2014
//  Copyright 2014 Tag Games. All rights reserved.
//

#include <ChilliSource/Backend/Platform/Android/Core/DialogueBox/DialogueBoxSystem.h>

#include <ChilliSource/Backend/Platform/Android/Core/DialogueBox/DialogueBoxJavaInterface.h>
#include <ChilliSource/Backend/Platform/Android/Core/JNI/JavaInterfaceManager.h>
#include <ChilliSource/Core/Base/Application.h>
#include <ChilliSource/Core/Base/PlatformSystem.h>

namespace ChilliSource
{
	namespace Android
	{
        CS_DEFINE_NAMEDTYPE(DialogueBoxSystem);
        //----------------------------------------------------
        //----------------------------------------------------
        DialogueBoxSystem::DialogueBoxSystem()
        {
        	m_dialogueBoxJI = JavaInterfaceManager::GetSingletonPtr()->GetJavaInterface<DialogueBoxJavaInterface>();
        	if (m_dialogueBoxJI == nullptr)
        	{
        		m_dialogueBoxJI = std::make_shared<DialogueBoxJavaInterface>();
        		JavaInterfaceManager::GetSingletonPtr()->AddJavaInterface(m_dialogueBoxJI);
        	}
        }
        //----------------------------------------------------
        //----------------------------------------------------
        bool DialogueBoxSystem::IsA(Core::InterfaceIDType in_interfaceID) const
        {
            return (DialogueBoxSystem::InterfaceID == in_interfaceID || Core::DialogueBoxSystem::InterfaceID == in_interfaceID);
        }
        //-----------------------------------------------------
        //-----------------------------------------------------
        void DialogueBoxSystem::ShowSystemDialogue(u32 in_id, const Core::DialogueBoxSystem::DialogueDelegate& in_delegate, const Core::UTF8String& in_title, const Core::UTF8String& in_message, const Core::UTF8String& in_confirm)
        {
        	m_dialogueBoxJI->ShowSystemDialogue(in_id, in_title, in_message, in_confirm);
            m_activeSysConfirmDelegate = in_delegate;
        }
        //-----------------------------------------------------
        //-----------------------------------------------------
        void DialogueBoxSystem::ShowSystemConfirmDialogue(u32 in_id, const Core::DialogueBoxSystem::DialogueDelegate& in_delegate, const Core::UTF8String& in_title, const Core::UTF8String& in_message, const Core::UTF8String& in_confirm, const Core::UTF8String& in_cancel)
        {
        	m_dialogueBoxJI->ShowSystemConfirmDialogue(in_id, in_title, in_message, in_confirm, in_cancel);
            m_activeSysConfirmDelegate = in_delegate;
        }
        //-----------------------------------------------------
        //-----------------------------------------------------
        void DialogueBoxSystem::MakeToast(const Core::UTF8String& in_text)
        {
        	m_dialogueBoxJI->MakeToast(in_text);
        }
        //------------------------------------------------------
        //------------------------------------------------------
        void DialogueBoxSystem::OnSystemConfirmDialogueResult(u32 in_id, Core::DialogueBoxSystem::DialogueResult in_result)
        {
            if(m_activeSysConfirmDelegate)
        	{
        		m_activeSysConfirmDelegate(in_id, in_result);
        		m_activeSysConfirmDelegate = nullptr;
        	}
        }
        //-----------------------------------------------------
        //-----------------------------------------------------
        DialogueBoxSystem::~DialogueBoxSystem()
        {
        }
	}
}