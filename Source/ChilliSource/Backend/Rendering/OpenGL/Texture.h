/*
 *  GLTexture.h
 *  moFlo
 *
 *  Created by Tag Games on 01/10/2010.
 *  Copyright 2010 Tag Games. All rights reserved.
 *
 */

#ifndef _MOFLOW_OPENGL_TEXTURE_H_
#define _MOFLOW_OPENGL_TEXTURE_H_

#include <ChilliSource/Core/Image.h>
#include <ChilliSource/RenderingPlatform/OpenGL/ForwardDeclarations.h>
#include <ChilliSource/RenderingPlatform/OpenGL/GLIncludes.h>
#include <ChilliSource/Rendering/ForwardDeclarations.h>
#include <ChilliSource/Rendering/Texture.h>

namespace moFlo
{
	namespace OpenGL
	{
		class CTexture : public moFlo::Rendering::ITexture
		{
		public:

			virtual ~CTexture();
			//--------------------------------------------------
			/// Init
			///
			/// @param Width
			/// @param Height
			//--------------------------------------------------
			void Init(u32 inudwWidth, u32 inudwHeight, Core::CImage::Format ineFormat);
			//--------------------------------------------------
			/// Init
			///
			/// @param Source image
			/// @param Whether to create mip maps
			//--------------------------------------------------
			void Init(Core::CImage * pSourceImage, bool inbWithMipsMaps);
            //----------------------------------------------------------------
            /// Create Image From Texture
            ///
            /// @param Image To Initialize from this Texture
            //----------------------------------------------------------------
            bool CreateImage(Core::ImagePtr& outpImage);
			//--------------------------------------------------
			/// Is A
			///
			/// @param Interface ID type
			/// @return Whether the object is of this type
			//--------------------------------------------------
			bool IsA(Core::InterfaceIDType inInterfaceID) const;
			//--------------------------------------------------
			/// Bind
			///
			/// @param Texture unit to bind texture to (default 0)
			//--------------------------------------------------
			void Bind(u32 inSlot = 0);
			//--------------------------------------------------
			/// Unbind
			///
			/// Unbind this texture from a slot if it is bound
			//--------------------------------------------------
			void Unbind();
			//--------------------------------------------------
			/// Set Filter
			///
			/// Set the filtering mode
			/// @param S filter mode
			/// @param T filter mode
			//--------------------------------------------------
			void SetFilter(Filter ineSFilter, Filter ineTFilter);
			//--------------------------------------------------
			/// Set Wrap Mode
			///
			/// Set the texture wrap mode
			/// @param S wrap mode
			/// @param T wrap mode
			//--------------------------------------------------
			void SetWrapMode(WrapMode inSWrapMode, WrapMode inTWrapMode);
			//--------------------------------------------------
			/// Get Texture ID
			///
			/// @return The GL generated texture handle
			//--------------------------------------------------
			GLuint GetTextureID() const;
            //--------------------------------------------------
            /// Has Mip Maps
            ///
            /// @return Whether or not this was told to use
            ///			mip maps
            //--------------------------------------------------
            bool HasMipMaps() const;
            //--------------------------------------------------
            /// Get Image Format
            ///
            /// @return The format of the image used to create
            ///			the texture.
            //--------------------------------------------------
            Core::CImage::Format GetImageFormat() const; 
            //--------------------------------------------------
            /// Reset
            ///
            /// Restores the texture to it's state prior to
            /// calling init. Init will need to be called
            /// again before the texture can be used.
            ///
            /// @param Object ID
            //--------------------------------------------------
            static void Reset(GLuint& inTextureID, u8* inpObjectID);
            //--------------------------------------------------
            /// Reset
            ///
            /// Restores the texture to it's state prior to
            /// calling init. Init will need to be called
            /// again before the texture can be used.
            //--------------------------------------------------
            void Reset();
            //---------------------------------------------------
            /// Set Active Texture Slot
            ///
            /// @param Texture slot
            //---------------------------------------------------
            static void SetActiveTextureSlot(u32 inudwSlot);
            //---------------------------------------------------
            /// Bind
            ///
            /// @param Texture type
            /// @param Texture slot
            /// @param ID of the texture occupying the slot
            /// @param ID of the object occupying the slot
            //---------------------------------------------------
            static void Bind(GLenum inType, u32 inudwSlot, GLint inTextureID, u8* inpObjectID);
            //--------------------------------------------------
            /// Unbind
            ///
            /// Unbind the object with the given ID
            ///
            /// @param Object ID
            //--------------------------------------------------
            static void Unbind(u8* inpObjectID);
			//--------------------------------------------------
			/// Clear Cache
			///
			/// Flush the currently bound texture cache
			//--------------------------------------------------
			static void ClearCache();
			
		protected:
			CTexture(CTextureManager* inpTextureManager);
            //---------------------------------------------------
            /// Error Check
            ///
            /// Throw a warning if the texture is non power of
            /// two or if it exceeds the maximum texture size
            ///
            /// @param Width
            /// @param Height
            //---------------------------------------------------
            void ErrorCheck(u32 inudwWidth, u32 inudwHeight);
            //---------------------------------------------------
            /// Update Texture Parameters
            ///
            /// Update the texture filter and repeat modes
            //---------------------------------------------------
            void UpdateTextureParameters();
			
			friend class CTextureManager;
			
		protected:
			GLuint mGLTexID;
			Filter meSFilter;
			Filter meTFilter;
			WrapMode meSWrapMode;
			WrapMode meTWrapMode;
            
            Core::CImage::Format meImageFormat;
            
			bool mbHasMipMaps;
            bool mbHasTextureFilterModeChanged;
            
            s32 mdwTextureSlot;

            CTextureManager* mpTextureManager;
            Rendering::IRenderCapabilities* mpRenderCapabilities;
            
            static u32 udwCurrentActiveSlot;
            
            struct TextureUnit
            {
                TextureUnit()
                : pObjectID(NULL), dwTextureSlot(-1)
                {
                    
                }
                u8* pObjectID;
                s32 dwTextureSlot;
            };
            
			static TextureUnit* paTextureUnits;
		};
	}
}

#endif