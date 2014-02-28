//
//  SceneDescManager.cpp
//  MoshiMonsters
//
//	
//
//  Created by Andrew Glass on 20/04/2012.
//  Copyright (c) 2012 Tag Games. All rights reserved.
//

#include <ChilliSource/Core/Scene/SceneDescManager.h>
#include <ChilliSource/Core/Scene/SceneDescription.h>
#include <ChilliSource/Core/Scene/SceneProvider.h>

namespace ChilliSource
{
    namespace Core
    {
        CS_DEFINE_NAMEDTYPE(SceneDescManager);
        
        bool SceneDescManager::IsA(InterfaceIDType inInterfaceID) const
        {
            return SceneDescManager::InterfaceID == inInterfaceID;
        }
        
        InterfaceIDType SceneDescManager::GetResourceType() const
        {
            return SceneDesc::InterfaceID;
        }
        
        InterfaceIDType SceneDescManager::GetProviderType() const
        {
            return SceneDesc::InterfaceID;
        }
        
        bool SceneDescManager::ManagesResourceOfType(InterfaceIDType inInterfaceID) const
        {
            return inInterfaceID == SceneDesc::InterfaceID;
        }
        
        ResourceSPtr SceneDescManager::GetResourceFromFile(ChilliSource::Core::StorageLocation ineStorageLocation, const std::string &instrFilePath)
        {
            return GetSceneDescFromFile(ineStorageLocation, instrFilePath);
        }
        
        ResourceSPtr SceneDescManager::AsyncGetResourceFromFile(ChilliSource::Core::StorageLocation ineStorageLocation, const std::string &instrFilePath)
        {
            return AsyncGetSceneDescFromFile(ineStorageLocation, instrFilePath);
        }
        
        SceneDescSPtr SceneDescManager::GetSceneDescFromFile(ChilliSource::Core::StorageLocation ineStorageLocation, const std::string &instrFilePath)
        {
            MapStringToResourceSPtr::iterator pExistingResource = mMapFilenameToResource.find(instrFilePath);
            
            if(pExistingResource == mMapFilenameToResource.end())
            {
                ResourceSPtr pResource(new SceneDesc());
                for(u32 nProvider = 0; nProvider < mResourceProviders.size(); nProvider++)
                {
                    if(mResourceProviders[nProvider]->CreateResourceFromFile(ineStorageLocation, instrFilePath, pResource))
                    {
                        CS_LOG_DEBUG("Loading Scene Description " + instrFilePath);
                        mMapFilenameToResource.emplace(instrFilePath, pResource);
                        
                        SceneDescSPtr pSceneDesc = std::static_pointer_cast<SceneDesc>(pResource);
                        pSceneDesc->SetName(instrFilePath);
                        pSceneDesc->SetOwningResourceManager(this);
                        pSceneDesc->SetFilename(instrFilePath);
                        pSceneDesc->SetStorageLocation(ineStorageLocation);
                        pSceneDesc->SetLoaded(true);
                        
                        return pSceneDesc;
                    }
                }
            }
            else
                return std::static_pointer_cast<SceneDesc>(pExistingResource->second);
            
            CS_LOG_ERROR("Cannot find resource for Scene Description with path " + instrFilePath);
            return SceneDescSPtr();
        }
        
        SceneDescSPtr SceneDescManager::AsyncGetSceneDescFromFile(ChilliSource::Core::StorageLocation ineStorageLocation, const std::string &instrFilePath)
        {
            MapStringToResourceSPtr::iterator pExistingResource = mMapFilenameToResource.find(instrFilePath);
            
            if(pExistingResource == mMapFilenameToResource.end())
            {
                ResourceSPtr pResource(new SceneDesc());
                for(u32 nProvider = 0; nProvider < mResourceProviders.size(); nProvider++)
                {
                    SceneDescSPtr pSceneDesc = std::static_pointer_cast<SceneDesc>(pResource);
                    pSceneDesc->SetName(instrFilePath);
                    pSceneDesc->SetOwningResourceManager(this);
                    pSceneDesc->SetFilename(instrFilePath);
                    pSceneDesc->SetStorageLocation(ineStorageLocation);
                    pSceneDesc->SetLoaded(false);

                    if(mResourceProviders[nProvider]->AsyncCreateResourceFromFile(ineStorageLocation, instrFilePath, pResource))
                    {
                        CS_LOG_DEBUG("Loading Scene Description " + instrFilePath);
                        mMapFilenameToResource.emplace(instrFilePath, pResource);
                                                
                        return pSceneDesc;
                    }
                }
            }
            else
                return std::static_pointer_cast<SceneDesc>(pExistingResource->second);
            
            CS_LOG_ERROR("Cannot find resource for Scene Description with path " + instrFilePath);
            return SceneDescSPtr();
        }
        
    }
}