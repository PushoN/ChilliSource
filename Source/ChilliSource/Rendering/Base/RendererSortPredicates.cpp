/*
 *  RendererSortPredicates.cpp
 *  MoFlow
 *
 *  Created by Stuart McGaw on 26/04/2011.
 *  Copyright 2011 Tag Games. All rights reserved.
 *
 */

#include <ChilliSource/Rendering/Base/RendererSortPredicates.h>
#include <ChilliSource/Rendering/Base/Renderer.h>
#include <ChilliSource/Rendering/Model/StaticMeshComponent.h>

#include <ChilliSource/Core/Entity/Entity.h>
#include <ChilliSource/Core/Entity/Transform.h>

using namespace ChilliSource::Core;

namespace ChilliSource
{
	namespace Rendering
    {
		void BackToFrontSortPredicate::PrepareForSort(std::vector<RenderComponent*> * inpRenderable)
        {
			mCameraViewProj = Renderer::matViewProjCache;
            
            Matrix4 matViewTrans;
            Matrix4 matWorld;
            
            for(u32 i = 0; i < inpRenderable->size(); ++i)
            {
				Core::Matrix4 matLocalTrans = Core::Matrix4::CreateTranslation((*inpRenderable)[i]->GetAABB().GetOrigin() - (*inpRenderable)[i]->GetEntity()->GetTransform().GetWorldPosition());
                matWorld = (*inpRenderable)[i]->GetTransformationMatrix() * matLocalTrans;
				matViewTrans = matWorld * mCameraViewProj;
                (*inpRenderable)[i]->SetSortValue(matViewTrans.GetTranslation().z);
            }
		}
        
		bool BackToFrontSortPredicate::SortItem(const RenderComponent* p1, const RenderComponent* p2) const
        {
			return p1->GetSortValue() > p2->GetSortValue();
		}
        
        bool MaterialSortPredicate::SortItem(const RenderComponent* p1, const RenderComponent* p2) const
        {
            const Material* pM1 = p1->GetMaterial().get();
            const Material* pM2 = p2->GetMaterial().get();
            if(pM1 == pM2)
            {
                bool bp1Mesh = p1->IsA(StaticMeshComponent::InterfaceID);
                bool bp2Mesh = p2->IsA(StaticMeshComponent::InterfaceID);
                if(bp1Mesh && bp2Mesh)
                {
                    return ((StaticMeshComponent*)p1)->GetMesh().get() < ((StaticMeshComponent*)p2)->GetMesh().get();
                }
                else
                {
                    return bp1Mesh;
                }
            }
            else
            {
                return p1->GetMaterial().get() < p2->GetMaterial().get();
            }
		}
	}
}
