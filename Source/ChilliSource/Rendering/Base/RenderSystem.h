/*
 *  IRenderSystem.h
 *  MoFlow
 *
 *  Created by Tag Games on 28/09/2010.
 *  Copyright 2010 Tag Games. All rights reserved.
 *
 */


#ifndef _MOFLO_RENDERING_IRENDERSYSTEM_H
#define _MOFLO_RENDERING_IRENDERSYSTEM_H

#include <ChilliSource/Core/System/System.h>
#include <ChilliSource/Core/System/SystemConcepts.h>
#include <ChilliSource/Core/Base/Colour.h>

#include <ChilliSource/Rendering/ForwardDeclarations.h>
#include <ChilliSource/Rendering/Lighting/LightComponent.h>
#include <ChilliSource/Rendering/Base/RenderTarget.h>
#include <ChilliSource/Rendering/Material/Material.h>
#include <ChilliSource/Rendering/Texture/TextureManager.h>
#include <ChilliSource/Rendering/Model/MeshManager.h>
#include <ChilliSource/Rendering/Model/SkinnedAnimationManager.h>
#include <ChilliSource/Rendering/Material/MaterialManager.h>
#include <ChilliSource/Rendering/Sprite/SpriteSheetManager.h>
#include <ChilliSource/Rendering/Font/FontManager.h>

namespace moFlo
{
	namespace Rendering
	{
        class CDynamicSpriteBatch;
        
		class IRenderSystem : public Core::ISystem, public Core::IComponentProducer
		{
		public:
			DECLARE_NAMED_INTERFACE(IRenderSystem);
			IRenderSystem();
			virtual ~IRenderSystem();
			
			virtual bool Init(u32 inudwWidth, u32 inudwHeight) = 0;
			virtual void Resume() = 0;
			virtual void Suspend() = 0;
			virtual void Destroy() = 0;
			virtual void OnScreenOrientationChanged(u32 inudwWidth, u32 inudwHeight) = 0;

			virtual void BeginFrame(IRenderTarget* inpActiveRenderTarget) = 0;
			virtual void EndFrame(IRenderTarget* inpActiveRenderTarget) = 0;

			virtual void ApplyMaterial(const Rendering::CMaterial& inMaterial) = 0;
            virtual void ApplyJoints(const DYNAMIC_ARRAY<Core::CMatrix4x4>& inaJoints) = 0;
			virtual void ApplyCamera(const Core::CVector3& invPosition, const Core::CMatrix4x4& inmatInvView, const Core::CMatrix4x4& inmatProj, const Core::CColour& inClearCol) = 0;
            virtual void SetLight(ILightComponent* inpLightComponent) = 0;
			
			virtual void EnableAlphaBlending(bool inbIsEnabled) = 0;
			virtual void EnableDepthTesting(bool inbIsEnabled) = 0;
			virtual void EnableFaceCulling(bool inbIsEnabled) = 0;
			virtual void EnableColourWriting(bool inbIsEnabled) = 0;
			virtual void EnableDepthWriting(bool inbIsEnabled) = 0;
            virtual void EnableScissorTesting(bool inbIsEnabled) = 0;
            
            virtual void LockDepthWriting() = 0;
            virtual void UnlockDepthWriting() = 0;
            virtual void LockAlphaBlending() = 0;
            virtual void UnlockAlphaBlending() = 0;
			
            virtual void LockBlendFunction() = 0;
            virtual void UnlockBlendFunction() = 0;
            
			virtual void SetBlendFunction(AlphaBlend ineSrcFunc, AlphaBlend ineDstFunc) = 0;
            virtual void SetDepthFunction(DepthFunction ineFunc) = 0;
            virtual void SetCullFace(CullFace ineCullFace) = 0;
            virtual void SetScissorRegion(const Core::CVector2& invPosition, const Core::CVector2& invSize) = 0;
			
			virtual Rendering::IMeshBuffer* CreateBuffer(BufferDescription&) = 0;
			virtual void RenderVertexBuffer(Rendering::IMeshBuffer*, u32 inudwOffset, u32 inudwStride, const Core::CMatrix4x4&) = 0;
			virtual void RenderBuffer(Rendering::IMeshBuffer*, u32 inudwOffset, u32 inudwStride, const Core::CMatrix4x4&) = 0;
			
			virtual Rendering::IRenderTarget* CreateRenderTarget(u32 inWidth, u32 inHeight) = 0;
			virtual Rendering::IRenderTarget* GetDefaultRenderTarget() = 0;
			
			Core::IComponentFactory* GetComponentFactoryPtr(u32 inudwIndex);
			Core::IComponentFactory& GetComponentFactory(u32 inudwIndex);
            
            virtual std::string GetPathToShaders() const = 0;
			
			//----------------------------------------------------
			/// Get Number Of Component Factories
			///
			/// @return Number of factories in this system
			//----------------------------------------------------
			u32 GetNumComponentFactories() const;
            //----------------------------------------------------
            /// Get Dynamic Sprite Batch Pointer
            ///
            /// @return Pointer to dynamic sprite batcher
            //----------------------------------------------------
            CDynamicSpriteBatch* GetDynamicSpriteBatchPtr();
		protected:
			
			//---Render Factories
			CRenderComponentFactory* mpRenderFactory;
			
			//---Render resource managers
			CMeshManager mMeshManager;
			CSkinnedAnimationManager mSkinnedAnimationManager;
            IFontManager mFontManager;
			IMaterialManager mMaterialManager;
			ISpriteSheetManager mSpriteManager;
            
            CDynamicSpriteBatch* mpSpriteBatcher;
		};
	}
}

#endif
