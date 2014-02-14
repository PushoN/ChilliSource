/*
 *  moFloApplication.cpp
 *  moFlo
 *
 *  Created by Scott Downie on 23/09/2010.
 *  Copyright 2010 Tag Games. All rights reserved.
 *
 */

#include <ChilliSource/Revision.h>

#include <ChilliSource/Core/Base/Application.h>
#include <ChilliSource/Core/Base/ApplicationEvents.h>
#include <ChilliSource/Core/Base/ApplicationDelegates.h>
#include <ChilliSource/Core/System/SystemConcepts.h>
#include <ChilliSource/Core/Resource/ResourceProvider.h>
#include <ChilliSource/Core/Entity/ComponentFactory.h>
#include <ChilliSource/Core/Time/CoreTimer.h>
#include <ChilliSource/Core/File/TweakableConstants.h>
#include <ChilliSource/Core/File/LocalDataStore.h>
#include <ChilliSource/Core/Localisation/LocalisedText.h>
#include <ChilliSource/Core/State/StateManager.h>
#include <ChilliSource/Core/Resource/ResourceGroupManager.h>
#include <ChilliSource/Core/ForwardDeclarations.h>
#include <ChilliSource/Core/Base/Screen.h>
#include <ChilliSource/Core/Base/Logging.h>
#include <ChilliSource/Core/Base/Device.h>
#include <ChilliSource/Core/System/System.h>
#include <ChilliSource/Core/Base/Utils.h>
#include <ChilliSource/Core/Resource/ResourceManager.h>
#include <ChilliSource/Core/Resource/ResourceManagerDispenser.h>
#include <ChilliSource/Core/Entity/ComponentFactoryDispenser.h>
#include <ChilliSource/Core/Threading/TaskScheduler.h>
#include <ChilliSource/Core/Base/PlatformSystem.h>
#include <ChilliSource/Core/Notifications/NotificationScheduler.h>

#include <ChilliSource/Audio/ForwardDeclarations.h>
#include <ChilliSource/Rendering/ForwardDeclarations.h>
#include <ChilliSource/Rendering/Material/Material.h>
#include <ChilliSource/Rendering/Font/Font.h>
#include <ChilliSource/Rendering/Model/Mesh.h>

#include <ctime>

namespace moFlo
{
	namespace Core
	{
        //---Const Definitions
		const f32 kfDefaultUpdateInterval	= 1.0f/60.0f;
		const f32 kfUpdateClampThreshold	= 0.33f;                    //The number of seconds after which time between frames is clamped
		const f32 kfUpdateIntervalMax		= kfUpdateClampThreshold;	//We never carry more than this to the next frame
        
        //---Static Definitions
        TimeIntervalSecs CApplication::uddwCurrentAppTime = 0;
		
        CStateManager CApplication::mStateMgr;
        Rendering::FontPtr CApplication::pDefaultFont;
        Rendering::MeshPtr CApplication::pDefaultMesh;
        Rendering::MaterialPtr CApplication::pDefaultMaterial;

        bool CApplication::mbHasTouchInput = false;
        bool CApplication::mbUpdateSystems = true;
        f32 CApplication::mfUpdateInterval = kfDefaultUpdateInterval;
        f32 CApplication::mfUpdateSpeed = 1.0f;
        
        Rendering::IRenderSystem* CApplication::mpRenderSystem = NULL;
        Input::IInputSystem * CApplication::mpInputSystem = NULL;
        IPlatformSystem* CApplication::pPlatformSystem = NULL;
		Audio::IAudioSystem* CApplication::pAudioSystem = NULL;
		Rendering::CRenderer* CApplication::mpRenderer = NULL;
		IFileSystem* CApplication::mspFileSystem = NULL;
        
        DYNAMIC_ARRAY<IUpdateable*> CApplication::mUpdateableSystems;
        DYNAMIC_ARRAY<SystemPtr> CApplication::mSystems;
        
        ScreenOrientation CApplication::meDefaultOrientation = LANDSCAPE_RIGHT;
        
        CResourceManagerDispenser* CApplication::mpResourceManagerDispenser = NULL;
		
        moFlo::IApplicationDelegates* CApplication::mpApplicationDelegates = NULL;

        SystemConfirmDialog::Delegate CApplication::mActiveSysConfirmDelegate;

		//--------------------------------------------------------------------------------------------------
		/// Constructor
		///
		/// Default
		//--------------------------------------------------------------------------------------------------
		CApplication::CApplication()
		{
            mpResourceManagerDispenser = new CResourceManagerDispenser(this);
            mpComponentFactoryDispenser = new CComponentFactoryDispenser(this);
            mStateMgr.SetOwningApplication(this);

#ifdef TARGET_WINDOWS
			//Because windows by default is landscape, this needs to be flipped.
			meDefaultOrientation = PORTRAIT_UP;
#endif
            
			pPlatformSystem = IPlatformSystem::Create();
			mpApplicationDelegates = moFlo::IApplicationDelegates::Create();
		}
        //--------------------------------------------------------------------------------------------------
        /// Resolution Sort Predicate
        ///
        /// Sorts by directory info resolution (lowest to highest)
        //--------------------------------------------------------------------------------------------------
        bool ResolutionSortPredicate(const ResourceDirectoryInfo& inLHS, const ResourceDirectoryInfo& inRHS)
        {
            return inLHS.udwMaxRes < inRHS.udwMaxRes;
        }
		//--------------------------------------------------------------------------------------------------
		/// Run
		///
		/// Launch the application's setup code and cause it to begin it's update loop
		//--------------------------------------------------------------------------------------------------
		void CApplication::Run()
		{
			//Initialise the platform specific API's
			pPlatformSystem->Init();
            
			//Set the screen helper classes dimensions
            Core::CScreen::SetRawDimensions(pPlatformSystem->GetScreenDimensions());
            Core::CScreen::SetOrientation(meDefaultOrientation);
            Core::CScreen::SetDensity(pPlatformSystem->GetScreenDensity());
            
            DetermineResourceDirectories();

            //init tweakable constants and local data store.
			new CTweakableConstants();
			new CLocalDataStore();

            //Set up the device helper
            CDevice::Init(pPlatformSystem);

			//Set up the task scheduler
			CTaskScheduler::Init(Core::CDevice::GetNumCPUCores() * 4);

			//System setup
			pPlatformSystem->CreateDefaultSystems(mSystems);
            
			CreateSystems();
			PostCreateSystems();
            LoadDefaultResources();
			OnScreenChangedOrientation(meDefaultOrientation);

			if (mStateMgr.GetActiveScenePtr() == NULL)
				PushInitialState();

			//Register for update events
			CLocalDataStore::GetSingleton().SubscribeToApplicationSuspendEvent();

			//Begin the update loop
			pPlatformSystem->Run();
		}
        //--------------------------------------------------------------------------------------------------
        /// Determine Resource Directories
        ///
        /// Depedending on the device decide which folders resources should be loaded from
        //--------------------------------------------------------------------------------------------------
        void CApplication::DetermineResourceDirectories()
        {
            //Get a list of the resource directories and determine which one this device should be
            //loading from based on it's screen
            DYNAMIC_ARRAY<ResourceDirectoryInfo> aDirectoryInfos;
            std::string strDefaultDir, strDeviceDir, strDefaultDeviceDir;
            SetResourceDirectories(aDirectoryInfos, strDefaultDeviceDir, strDefaultDir);
            
            //Sort the info by resolution low to high
            std::sort(aDirectoryInfos.begin(), aDirectoryInfos.end(), ResolutionSortPredicate);
            
            u32 udwCurrentRes = CScreen::GetOrientedWidth() * CScreen::GetOrientedHeight();
            f32 fCurrenctDensity = CScreen::GetDensity();
            f32 fAssetDensity = 1.0f;
            for(DYNAMIC_ARRAY<ResourceDirectoryInfo>::const_iterator it = aDirectoryInfos.begin(); it != aDirectoryInfos.end(); ++it)
            {
                //The density and the resolution must both be under the maximum for the directory to be selected.
                if(udwCurrentRes <= it->udwMaxRes && fCurrenctDensity <= it->fMaxDensity)
                {
                    strDeviceDir = it->strDirectory;
                    fAssetDensity = it->fResourcesDensity;
                    break;
                }
            }
            
            if(strDeviceDir.empty())
            {
                WARNING_LOG("No resource folder can be found for this device switching to default directory");
                strDeviceDir = strDefaultDeviceDir;
            }
            
            IFileSystem::SetResourceDirectories(strDeviceDir, strDefaultDeviceDir, strDefaultDir, fAssetDensity);
        }
		//--------------------------------------------------------------------------------------------------
		/// Get System Implementing
		///
		/// Looks for a system that implements the given interface (i.e. a 2DRenderSystem etc)
		/// @param The type ID of the system you wish to implement
		/// @return System that implements the given interface or NULL if no system
		//--------------------------------------------------------------------------------------------------
		SystemPtr CApplication::GetSystemImplementing(InterfaceIDType inInterfaceID)
		{
			for (DYNAMIC_ARRAY<SystemPtr>::const_iterator it = mSystems.begin(); it != mSystems.end(); ++it) 
			{
				if ((*it)->IsA(inInterfaceID)) 
				{
					return (*it);
				}
			}
			
			WARNING_LOG("Application cannot find implementing systems");
			return SystemPtr();
		}
		//--------------------------------------------------------------------------------------------------
		/// Get Systems Implementing
		///
		/// Looks for systems that implements the given interface (i.e. a 2DRenderSystem etc)
		/// and fills an array with them.
		/// @param The type ID of the system you wish to implement
		//--------------------------------------------------------------------------------------------------
		void CApplication::GetSystemsImplementing(InterfaceIDType inInterfaceID, DYNAMIC_ARRAY<SystemPtr> & outSystems)
		{
			outSystems.clear();
			for (DYNAMIC_ARRAY<SystemPtr>::const_iterator it = mSystems.begin(); it != mSystems.end(); ++it) 
			{
				if ((*it)->IsA(inInterfaceID)) 
				{
					outSystems.push_back((*it));
				}
			}
			
			WARNING_LOG("Application cannot find implementing systems");
		}
		//--------------------------------------------------------------------------------------------------
		/// Get Resource Provider Producing
		///
		/// Looks for a resource provider that can create a resource of type
		/// @param The type ID of the resource you wish to create (i.e. Model, Texture)
		/// @return Resource provider that loads the resource type
		//--------------------------------------------------------------------------------------------------
		IResourceProvider* CApplication::GetResourceProviderProducing(InterfaceIDType inInterfaceID, const std::string & inExtension)
		{
			for (DYNAMIC_ARRAY<IResourceProvider*>::iterator pProv = mResourceProviders.begin(); pProv != mResourceProviders.end(); ++pProv) 
			{
				if ((*pProv)->CanCreateResourceFromFileWithExtension(inExtension)) 
				{
					return (*pProv);
				}
			}
			
			WARNING_LOG("Application cannot find resource provider");
			return NULL;
		}
        //--------------------------------------------------------------------------------------------------
        /// Get State Manager
        ///
        /// @return Application state manager
        //--------------------------------------------------------------------------------------------------
        CStateManager& CApplication::GetStateManager() 
        {
            return mStateMgr;
        }
        //--------------------------------------------------------------------------------------------------
        /// Get State Manager Pointer
        ///
        /// @return Handle to application state manager
        //--------------------------------------------------------------------------------------------------
        CStateManager* CApplication::GetStateManagerPtr() 
        {
            return &mStateMgr;
        }
		//--------------------------------------------------------------------------------------------------
		/// Get App Elapsed Time
		///
		/// @return The time elapsed since the application began
		//--------------------------------------------------------------------------------------------------
		TimeIntervalSecs CApplication::GetAppElapsedTime() 
		{
			return uddwCurrentAppTime;
		}
		//--------------------------------------------------------------------------------------------------
		/// Set App Elapsed Time
		///
		/// @param The time elapsed since the application began
		//--------------------------------------------------------------------------------------------------
		void CApplication::SetAppElapsedTime(TimeIntervalSecs inElapsedTime)
		{
			uddwCurrentAppTime = inElapsedTime;
		}
        //--------------------------------------------------------------------------------------------------
        /// Get System Time
        ///
        /// @return System clock time in seconds since epoch
        //--------------------------------------------------------------------------------------------------
        TimeIntervalSecs CApplication::GetSystemTime()
        {
            return time(0);
        }
		//--------------------------------------------------------------------------------------------------
		/// Get System Time In Milliseconds
		///
		/// @return System clock time in milliseconds since epoch
		//--------------------------------------------------------------------------------------------------
		TimeIntervalMs CApplication::GetSystemTimeInMilliseconds()
		{
			return pPlatformSystem->GetSystemTimeMS();
		}
        //--------------------------------------------------------------------------------------------------
		/// Get MoFlow Version
		///
		/// @return String containing the engine version number
		//--------------------------------------------------------------------------------------------------
		std::string CApplication::GetMoFlowVersion()
		{
			return MOFLOW_REVISION_NUMBER;
		}
		//--------------------------------------------------------------------------------------------------
		/// Get App Version
		///
		/// @return String containing the application version number
		//--------------------------------------------------------------------------------------------------
		std::string CApplication::GetAppVersion()
		{
			return APP_REVISION_NUMBER;
		}
		//--------------------------------------------------------------------------------------------------
		/// Post Create Systems
		///
		/// Once the systems have been created they are then added to the pool and initialised
		//--------------------------------------------------------------------------------------------------
		void CApplication::PostCreateSystems()
		{
            //Loop round all the created systems and categorise them
			for(DYNAMIC_ARRAY<SystemPtr>::iterator it = mSystems.begin(); it != mSystems.end(); ++it) 
			{
				if ((*it)->IsA(IUpdateable::InterfaceID))
				{
					mUpdateableSystems.push_back((*it)->GetInterface<IUpdateable>());
				}
				if ((*it)->IsA(IComponentProducer::InterfaceID)) 
				{
                    IComponentProducer* pProducer = (*it)->GetInterface<IComponentProducer>();
                    u32 udwNumFactoriesInSystem = pProducer->GetNumComponentFactories();

                    for(u32 i=0; i<udwNumFactoriesInSystem; ++i)
                    {
                        mpComponentFactoryDispenser->RegisterComponentFactory(pProducer->GetComponentFactoryPtr(i));
                    }
				}
				if ((*it)->IsA(IResourceProvider::InterfaceID)) 
				{
					mResourceProviders.push_back((*it)->GetInterface<IResourceProvider>());
				}
			}

            //Give the resource managers their providers
            mpResourceManagerDispenser->SetResourceProviders(mResourceProviders);

            pPlatformSystem->PostCreateSystems();
		}
        //-----------------------------------------
		/// Load Default Resources
		//-----------------------------------------
        void CApplication::LoadDefaultResources()
        {
            Json::Value jRoot;
            if(CUtils::ReadJson(SL_PACKAGE, "App.config", &jRoot) == true)
            {
                if(jRoot.isMember("MaxFPS"))
                {
                    u32 udwMaxFPS = jRoot["MaxFPS"].asUInt();
                    pPlatformSystem->SetMaxFPS(udwMaxFPS);
                }
                
                if(jRoot.isMember("MasterText"))
                {
                    STORAGE_LOCATION eStorageLocation = GetStorageLocationFromString(jRoot["MasterText"].get("Location", "Package").asString());
                    std::string strPath = jRoot["MasterText"].get("Path", "").asString();
                    RefreshMasterText(eStorageLocation, strPath);
                }
                
                if(jRoot.isMember("DefaultMesh"))
                {
                    STORAGE_LOCATION eStorageLocation = GetStorageLocationFromString(jRoot["DefaultMesh"].get("Location", "Package").asString());
                    std::string strPath = jRoot["DefaultMesh"].get("Path", "").asString();
                    pDefaultMesh = LOAD_RESOURCE(Rendering::CMesh, eStorageLocation, strPath);
                }
                
                if(jRoot.isMember("DefaultFont"))
                {
                    STORAGE_LOCATION eStorageLocation = GetStorageLocationFromString(jRoot["DefaultFont"].get("Location", "Package").asString());
                    std::string strPath = jRoot["DefaultFont"].get("Path", "").asString();
                    pDefaultFont = LOAD_RESOURCE(Rendering::CFont, eStorageLocation, strPath);
                }
                
                if(jRoot.isMember("DefaultMaterial"))
                {
                    STORAGE_LOCATION eStorageLocation = GetStorageLocationFromString(jRoot["DefaultMaterial"].get("Location", "Package").asString());
                    std::string strPath = jRoot["DefaultMaterial"].get("Path", "").asString();
                    pDefaultMaterial = LOAD_RESOURCE(Rendering::CMaterial, eStorageLocation, strPath);
                }
            }
        }
		//-----------------------------------------
		/// Set Update Interval
		///
		/// Reset the time between update calls
		/// to adjust the frame rate.
		///
		/// @param Time between update calls
		//-----------------------------------------
		void CApplication::SetUpdateInterval(f32 infUpdateInterval)
		{
			mfUpdateInterval = infUpdateInterval;			
		}
		//--------------------------------------------------------------------------------------------------
		/// Get Update Interval
		///
		/// @return Time between update calls
		//--------------------------------------------------------------------------------------------------
		f32 CApplication::GetUpdateInterval()
		{
			return mfUpdateInterval;
		}
		//--------------------------------------------------------------------------------------------------
		/// Get Max Update Interval
		///
		/// @return Max time to be processed in a single frame.
		//--------------------------------------------------------------------------------------------------
		f32 CApplication::GetUpdateIntervalMax()
		{
			return kfUpdateIntervalMax;
		}
        //--------------------------------------------------------------------------------------------------
        /// Set Update Speed
        ///
        /// @param Scaler to speed up or slow down update time
        //--------------------------------------------------------------------------------------------------
        void CApplication::SetUpdateSpeed(f32 infSpeed)
        {
            mfUpdateSpeed = infSpeed;
        }
        //--------------------------------------------------------------------------------------------------
        /// Refresh Master Text
        ///
        /// Call this after a DLC update to refresh the master text cache from file
        ///
        /// @param Storage location
        /// @param File path excluding name (i.e. if root then "")
        //--------------------------------------------------------------------------------------------------
        void CApplication::RefreshMasterText(STORAGE_LOCATION ineStorageLocation, const std::string& instrDirectory)
        {
            //Load any localised text from file 
            if(!Core::CLocalisedText::LoadTextFromFile(ineStorageLocation, instrDirectory, Core::CDevice::GetLanguage().GetLanguageCode() + ".mofloloca"))
            {
                //Default to english
                Core::CLocalisedText::LoadTextFromFile(ineStorageLocation, instrDirectory, "en.mofloloca");
            }
        }
        //--------------------------------------------------------------------------------------------------
        /// Make Toast
        ///
        /// Display a toast notification with the given text
        ///
        /// @param Text
        //--------------------------------------------------------------------------------------------------
        void CApplication::MakeToast(const UTF8String& instrText)
        {
            pPlatformSystem->MakeToast(instrText);
        }
        //--------------------------------------------------------------------------------------------------
        /// Get Default Font
        ///
        /// @return Default font given to the system by the application delegate
        //--------------------------------------------------------------------------------------------------
        const Rendering::FontPtr& CApplication::GetDefaultFont()
        {
            return pDefaultFont;
        }
        //--------------------------------------------------------------------------------------------------
        /// Get Default Mesh
        ///
        /// @return Default mesh given to the system by the application delegate
        //--------------------------------------------------------------------------------------------------
        const Rendering::MeshPtr& CApplication::GetDefaultMesh()
        {
            return pDefaultMesh;
        }
        //--------------------------------------------------------------------------------------------------
        /// Get Default Material
        ///
        /// @return Default material given to the system by the application delegate
        //--------------------------------------------------------------------------------------------------
        const Rendering::MaterialPtr& CApplication::GetDefaultMaterial()
        {
            return pDefaultMaterial;
        }
        //-----------------------------------------
        /// Quit
        ///
        /// Stop the application and exit 
        /// gracefully
        //-----------------------------------------
        void CApplication::Quit()
        {
            pPlatformSystem->TerminateUpdater();
        }
        //--------------------------------------------------------------------------------------------------
        /// Show System Confirm Dialog
        ///
        /// Display a system confirmation dialog with the given ID and delegate
        ///
        /// @param ID
        /// @param SystemConfirmDialogDelegate
        /// @param Title text
        /// @param Message text
        /// @param Confirm text
        /// @param Cancel text
        //--------------------------------------------------------------------------------------------------
        void CApplication::ShowSystemConfirmDialog(u32 inudwID, const SystemConfirmDialog::Delegate& inDelegate, const UTF8String& instrTitle, const UTF8String& instrMessage, const UTF8String& instrConfirm, const UTF8String& instrCancel)
        {
        	 pPlatformSystem->ShowSystemConfirmDialog(inudwID, instrTitle, instrMessage, instrConfirm, instrCancel);
        	 mActiveSysConfirmDelegate = inDelegate;
        }
        //--------------------------------------------------------------------------------------------------
        /// Show System Dialog
        ///
        /// Display a system dialog with the given ID and delegate
        ///
        /// @param ID
        /// @param SystemConfirmDialogDelegate
        /// @param Title text
        /// @param Message text
        /// @param Confirm text
        //--------------------------------------------------------------------------------------------------
        void CApplication::ShowSystemDialog(u32 inudwID, const SystemConfirmDialog::Delegate& inDelegate, const UTF8String& instrTitle, const UTF8String& instrMessage, const UTF8String& instrConfirm)
        {
            pPlatformSystem->ShowSystemDialog(inudwID, instrTitle, instrMessage, instrConfirm);
            mActiveSysConfirmDelegate = inDelegate;
        }
        //--------------------------------------------------------------------------------------------------
        /// On System Confirm Dialog Result
        ///
        /// Triggered from a system dialog confirmation event
        ///
        /// @param ID
        /// @param Result
        //--------------------------------------------------------------------------------------------------
        void CApplication::OnSystemConfirmDialogResult(u32 inudwID, SystemConfirmDialog::Result ineResult)
        {
        	if(mActiveSysConfirmDelegate)
        	{
        		mActiveSysConfirmDelegate(inudwID, ineResult);
        		mActiveSysConfirmDelegate = NULL;
        	}
        }
        //--------------------------------------------------------------------------------------------------
		/// Set Render System
		///
		/// @param the system pointer.
		//--------------------------------------------------------------------------------------------------
		void CApplication::SetRenderSystem(Rendering::IRenderSystem* inpSystem)
		{
			mpRenderSystem = inpSystem;
		}
        //--------------------------------------------------------------------------------------------------
		/// Set Input System
		///
		/// @param the system pointer.
		//--------------------------------------------------------------------------------------------------
		void CApplication::SetInputSystem(Input::IInputSystem* inpSystem)
		{
			mpInputSystem = inpSystem;
		}
        //--------------------------------------------------------------------------------------------------
		/// Set Audio System
		///
		/// @param the system pointer.
		//--------------------------------------------------------------------------------------------------
		void CApplication::SetAudioSystem(Audio::IAudioSystem* inpSystem)
		{
			pAudioSystem = inpSystem;
		}
        //--------------------------------------------------------------------------------------------------
		/// Set Renderer
		///
		/// @param the renderer
		//--------------------------------------------------------------------------------------------------
		void CApplication::SetRenderer(Rendering::CRenderer* inpSystem)
		{
			mpRenderer = inpSystem;
		}
		//--------------------------------------------------------------------------------------------------
		/// Set File System
		///
		/// @param the file system
		//--------------------------------------------------------------------------------------------------
		void CApplication::SetFileSystem(IFileSystem* inpSystem)
		{
			mspFileSystem = inpSystem;
			mSystems.push_back(SystemPtr(mspFileSystem));
		}
		 //--------------------------------------------------------------------------------------------------
		/// Set Has Touch Input
		///
		/// @param whether or not touch input is available.
		//--------------------------------------------------------------------------------------------------
		void CApplication::SetHasTouchInput(bool inbTouchInput)
		{
			mbHasTouchInput = inbTouchInput;
		}
		//--------------------------------------------------------------------------------------------------
		/// Has Touch Input
		///
		/// @return whether or not touch input is available.
		//--------------------------------------------------------------------------------------------------
		bool CApplication::HasTouchInput()
		{
			return mbHasTouchInput;
		}
		//--------------------------------------------------------------------------------------------------
		/// On Frame Begin
		///
		/// Triggered on receiving a "frame started" message. This will then update the state manager
		/// and each updatable subsystem with the time since last frame. It will also update any
		/// game timers.
		//--------------------------------------------------------------------------------------------------
		void CApplication::OnFrameBegin(f32 infDt, TimeIntervalSecs inuddwTimestamp)
		{
			mpApplicationDelegates->OnFrameBegin(infDt, inuddwTimestamp);
		}
        //--------------------------------------------------------------------------------------------------
        /// Update
        ///
        /// A single update cycle that updates all updateables, timers and the active state
        /// This can be called multiple times per frame depending on fixed updates.
        ///
        /// @param Time between frames
        //--------------------------------------------------------------------------------------------------
		void CApplication::Update(f32 infDT)
		{
            infDT *= mfUpdateSpeed;
            
			CCoreTimer::Update(infDT);
            
            CNotificationScheduler::Update(infDT);
            
			//Update sub systems
            if (mbUpdateSystems == true)
            {
				for(DYNAMIC_ARRAY<IUpdateable*>::iterator it = mUpdateableSystems.begin(); it != mUpdateableSystems.end(); ++it)
				{
					(*it)->Update(infDT);
				}
            }
			
			//Tell the state manager to update the active state
			mStateMgr.Update(infDT);
		}
		//--------------------------------------------------------------------------------------------------
		/// On Application Memory Warning
		///
		/// Triggered on receiving a "application memory warning" message.
		/// This will notify active resource managers to purge their caches
		//--------------------------------------------------------------------------------------------------
		void CApplication::OnApplicationMemoryWarning()
		{
			mpApplicationDelegates->OnApplicationMemoryWarning();
		}
		//--------------------------------------------------------------------------------------------------
		/// On Go Back
		///
		/// Triggered on receiving a "go back" event. This is usually caused by a back button being pressed
		//--------------------------------------------------------------------------------------------------
		void CApplication::OnGoBack()
		{
			mpApplicationDelegates->OnGoBack();
		}
		//----------------------------------------------------------------------
		/// Set Orientation
		/// 
		/// Tell the active camera to rotate its view and if we are using
		/// touch input we must rotate the input co-ordinates
		/// @param Screen orientation flag
		//----------------------------------------------------------------------
		void CApplication::SetOrientation(ScreenOrientation inOrientation)
		{
			mpApplicationDelegates->OnSetOrientation(inOrientation);
		}
		//--------------------------------------------------------------------------------------------------
		/// Enable System Updating
		///
		/// Enabled and disabled the updating of "updateable" systems.
		///
		/// @param whether or not to update.
		//--------------------------------------------------------------------------------------------------
		void CApplication::EnableSystemUpdating(bool inbEnable)
		{
			mbUpdateSystems = inbEnable;
		}
		//--------------------------------------------------------------------------------------------------
		/// Suspend
		///
        /// Triggered on receiving a "application will suspend" message.
		/// This will notify active states to pause and tell the sub systems to stop
		//--------------------------------------------------------------------------------------------------
		void CApplication::Suspend()
		{
            mpApplicationDelegates->OnApplicationSuspended();
		}
		//--------------------------------------------------------------------------------------------------
		/// Resume
		///
		/// Resumes application from suspended state
		//--------------------------------------------------------------------------------------------------
		void CApplication::Resume()
		{
            mpApplicationDelegates->SetInvokeResumeEvent(true);

			//We must restart the application timer. This will automatically restart system updates
			pPlatformSystem->SetUpdaterActive(true);
		}
		//--------------------------------------------------------------------------------------------------
		/// On Screen Resized
		///
		/// Triggered on receiving a "screen resized" message
		//--------------------------------------------------------------------------------------------------
		void CApplication::OnScreenResized(u32 inudwWidth, u32 inudwHeight) 
		{	
			mpApplicationDelegates->OnScreenResized(inudwWidth, inudwHeight);
		}
		//--------------------------------------------------------------------------------------------------
		/// On Screen Changed Orientation
		///
		/// Triggered on receiving a "orientation changed" message. Used to tell the camera and input
		/// to rotate
		//--------------------------------------------------------------------------------------------------
		void CApplication::OnScreenChangedOrientation(ScreenOrientation ineOrientation) 
		{		
			mpApplicationDelegates->OnScreenChangedOrientation(ineOrientation);
		}
		//--------------------------------------------------------------------------------------------------
		/// Destructor
		///
		//--------------------------------------------------------------------------------------------------
		CApplication::~CApplication()
		{
			mStateMgr.DestroyAll();

			pDefaultFont.reset();
			pDefaultMesh.reset();
			pDefaultMaterial.reset();

			SAFE_DELETE(pPlatformSystem);
            SAFE_DELETE(mpResourceManagerDispenser);
            SAFE_DELETE(mpComponentFactoryDispenser);

			//We have an issue with the order of destruction of systems.
			while(mSystems.empty() == false)
			{
				SystemPtr pSystem = mSystems.back();
				mSystems.pop_back();
				pSystem.reset();
			}
		}
	}
}