//
//  SceneProvider.cpp
//  MoshiMonsters
//
//	
//
//  Created by Andrew Glass on 20/04/2012.
//  Copyright (c) 2012 Tag Games. All rights reserved.
//

#include <ChilliSource/Core/SceneAnimation/SceneProvider.h>
#include <ChilliSource/Core/SceneAnimation/SceneDescription.h>
#include <ChilliSource/Core/TaskScheduler.h>
#include <ChilliSource/Core/Tasks.hpp>

namespace moFlo
{
    namespace Core
    {
        DEFINE_NAMED_INTERFACE(CSceneProvider);
        
        const std::string kMoSceneExtension("moscene");
        
        bool CSceneProvider::IsA(InterfaceIDType inInterface) const
        {
            return CSceneProvider::InterfaceID == inInterface||(inInterface) == IResourceProvider::InterfaceID;
        }
        
        
        bool CSceneProvider::CanCreateResourceOfKind(InterfaceIDType  inInterfaceID) const
        {
            return inInterfaceID == moFlo::Core::CSceneDesc::InterfaceID;
        }
        
        bool CSceneProvider::CanCreateResourceFromFileWithExtension(const std::string &inExtension) const
        {
            return inExtension == kMoSceneExtension;
        }
        
        bool CSceneProvider::CreateResourceFromFile(STORAGE_LOCATION ineStorageLocation, const std::string & inFilePath, ResourcePtr& outpResource)
        {
            return LoadMoScene(ineStorageLocation, inFilePath, outpResource);
        }
        
        bool CSceneProvider::AsyncCreateResourceFromFile(STORAGE_LOCATION ineStorageLocation, const std::string & inFilePath, ResourcePtr& outpResource)
        {
            Task3<STORAGE_LOCATION, const std::string&, ResourcePtr&> ReadFileTask(this, &CSceneProvider::LoadAsyncMoScene, ineStorageLocation, inFilePath, outpResource);
            CTaskScheduler::ScheduleTask(ReadFileTask);
            return true;
        }
        
        bool CSceneProvider::LoadMoScene(STORAGE_LOCATION ineStorageLocation, const std::string & inFilePath, ResourcePtr& outpResource)
        {
            CSceneDesc * pSceneDesc = reinterpret_cast<CSceneDesc*>(outpResource.get());
            
            return pSceneDesc->LoadFromFile(ineStorageLocation, inFilePath);
        }
        
        void CSceneProvider::LoadAsyncMoScene(STORAGE_LOCATION ineStorageLocation, const std::string & inFilePath, ResourcePtr& outpResource)
        {
            CSceneDesc * pSceneDesc = reinterpret_cast<CSceneDesc*>(outpResource.get());
            pSceneDesc->LoadFromFile(ineStorageLocation, inFilePath);
            outpResource->SetLoaded(true);
        }
    }
}