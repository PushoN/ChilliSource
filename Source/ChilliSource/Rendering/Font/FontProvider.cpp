//
//  FontProvider.cpp
//  Chilli Source
//
//  Created by Scott Downie on 26/10/2010.
//  Copyright 2010 Tag Games Ltd. All rights reserved.
//

#include <ChilliSource/Rendering/Font/FontProvider.h>

#include <ChilliSource/Core/Base/Application.h>
#include <ChilliSource/Core/Base/Utils.h>
#include <ChilliSource/Core/Resource/ResourceManagerDispenser.h>
#include <ChilliSource/Rendering/Font/Font.h>
#include <ChilliSource/Rendering/Sprite/SpriteSheet.h>
#include <ChilliSource/Rendering/Sprite/SpriteSheetManager.h>

namespace ChilliSource
{
	namespace Rendering
	{
        namespace
        {
            const std::string k_charFileExtension("alphabet");
            const std::string k_kerningFileExtension("kerninginfo");
            const std::string k_spriteSheetFileExtension("bin");
            
            //----------------------------------------------------------------------------
            /// Load the font kerning information from the external file
            ///
            /// @author R Henning
			///
            /// @param The storage location to load from
			/// @param Filename of font
			/// @param [Out] Pointer to font resource
            //----------------------------------------------------------------------------
            void LoadKerningInfo(const Core::FileStreamSPtr& in_kerningStream, Font* out_font)
            {
                u32 numKerningPairs;
                in_kerningStream->Read((s8*)&numKerningPairs, sizeof(u32));
                numKerningPairs = Core::Utils::Endian4ByteSwap(&numKerningPairs);
                
                // Kerning stored in a first character look up register
                std::vector<Font::KernLookup> kernLookups;
                std::vector<Font::KernPair> kernPairs;
                kernPairs.reserve(numKerningPairs);
                
                for(u32 pair=0; pair<numKerningPairs; ++pair)
                {
                    s16 char1;
                    in_kerningStream->Read((s8*)&char1, sizeof(s16));
                    char1 = Core::Utils::Endian2ByteSwap(&char1);
                    
                    s16 char2;
                    in_kerningStream->Read((s8*)&char2, sizeof(s16));
                    char2 = Core::Utils::Endian2ByteSwap(&char2);
                    
                    s16 spacing;
                    in_kerningStream->Read((s8*)&spacing, sizeof(s16));
                    spacing = Core::Utils::Endian2ByteSwap(&spacing);
                    
                    // Check if we are on a new c1
                    if(kernLookups.empty())
                    {
                        kernLookups.push_back(Font::KernLookup(char1, kernPairs.size()));
                    }
                    else if(kernLookups.back().m_character != char1)
                    {
                        kernLookups.back().m_length = kernPairs.size() - kernLookups.back().m_start;
                        kernLookups.push_back(Font::KernLookup(char1, kernPairs.size()));
                    }
                    kernPairs.push_back(Font::KernPair(char2, static_cast<f32>(spacing)));
                }
                
                out_font->SetKerningInfo(kernLookups, kernPairs);
            }
        }
		
        CS_DEFINE_NAMEDTYPE(FontProvider);
        
        //-------------------------------------------------------
        //-------------------------------------------------------
        FontProviderUPtr FontProvider::Create()
        {
            return FontProviderUPtr(new FontProvider());
        }
		//-------------------------------------------------------------------------e
		//-------------------------------------------------------------------------
		bool FontProvider::IsA(Core::InterfaceIDType in_interfaceId) const
		{
			return in_interfaceId == ResourceProvider::InterfaceID || in_interfaceId == FontProvider::InterfaceID;
		}
        //----------------------------------------------------------------------------
        //----------------------------------------------------------------------------
        Core::InterfaceIDType FontProvider::GetResourceType() const
        {
            return Font::InterfaceID;
        }
		//----------------------------------------------------------------------------
		//----------------------------------------------------------------------------
		bool FontProvider::CanCreateResourceWithFileExtension(const std::string& in_extension) const
		{
			return in_extension == k_charFileExtension;
		}
		//----------------------------------------------------------------------------
		//----------------------------------------------------------------------------
		void FontProvider::CreateResourceFromFile(Core::StorageLocation in_location, const std::string& in_filePath, Core::ResourceSPtr& out_resource)
		{
            LoadFont(in_location, in_filePath, nullptr, out_resource);
		}
        //----------------------------------------------------------------------------
        //----------------------------------------------------------------------------
        void FontProvider::CreateResourceFromFileAsync(Core::StorageLocation in_location, const std::string& in_filePath, const Core::ResourceProvider::AsyncLoadDelegate& in_delegate, Core::ResourceSPtr& out_resource)
        {
            Core::Task<Core::StorageLocation, const std::string&, const Core::ResourceProvider::AsyncLoadDelegate&, Core::ResourceSPtr&>
            task(this, &FontProvider::LoadFont, in_location, in_filePath, in_delegate, out_resource);
            Core::TaskScheduler::ScheduleTask(task);
        }
        //----------------------------------------------------------------------------
        //----------------------------------------------------------------------------
        void FontProvider::LoadFont(Core::StorageLocation in_location, const std::string& in_filePath, const Core::ResourceProvider::AsyncLoadDelegate& in_delegate, Core::ResourceSPtr& out_resource)
        {
            //TODO: We will be combining a font resource into a single .font file. In the meantime we
            //have to manually load the equivalent image file, data file and kerning file.
            
            Font* font = (Font*)(out_resource.get());
            
            std::string fileName, fileExtension;
            Core::StringUtils::SplitBaseFilename(in_filePath, fileName, fileExtension);
            const std::string alphabetFilePath(fileName + "." + k_charFileExtension);
			Core::FileStreamSPtr characterStream = Core::Application::Get()->GetFileSystem()->CreateFileStream(in_location, alphabetFilePath, Core::FileMode::k_read);
            if(characterStream == nullptr || characterStream->IsBad())
            {
                out_resource->SetLoadState(Core::Resource::LoadState::k_failed);
                if(in_delegate != nullptr)
                {
                    Core::Task<Core::ResourceSPtr&> task(in_delegate, out_resource);
                    Core::TaskScheduler::ScheduleMainThreadTask(task);
                }
                return;
            }
            
            //Buffer for the character set
            std::string characters;
            characterStream->GetLine(characters);
            characterStream->Close();
            font->SetCharacterSet(characters);
            
            // Get the kerning file - optional
            const std::string kerningFilePath(fileName + "." + k_kerningFileExtension);
            Core::FileStreamSPtr kerningStream = Core::Application::Get()->GetFileSystem()->CreateFileStream(in_location, kerningFilePath, Core::FileMode::k_read);
            
            if(kerningStream != nullptr && kerningStream->IsBad() == false)
            {
                LoadKerningInfo(kerningStream, font);
            }
            
            SpriteSheetSPtr fontData = LOAD_RESOURCE(SpriteSheet, in_location, in_filePath);
            font->SetCharacterData(fontData);
            
            if(fontData != nullptr)
            {
                out_resource->SetLoadState(Core::Resource::LoadState::k_loaded);
            }
            
            if(in_delegate != nullptr)
            {
                Core::Task<Core::ResourceSPtr&> task(in_delegate, out_resource);
                Core::TaskScheduler::ScheduleMainThreadTask(task);
            }
        }
	}
}
