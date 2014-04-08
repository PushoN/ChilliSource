//
//  Font.cpp
//  Chilli Source
//
//  Created by Scott Downie on 26/10/2010.
//  Copyright (c) 2010 Tag Games. All rights reserved.
//

#include <ChilliSource/Rendering/Font/Font.h>

#include <ChilliSource/Core/String/UTF8String.h>
#include <ChilliSource/Rendering/Sprite/SpriteSheet.h>

#include <algorithm>

namespace ChilliSource
{
	namespace Rendering
	{
        namespace
        {
            const Core::UTF8String::Char k_similarSpaceCharacter = 'n';
            const u32 k_spacesPerTab = 5;
        }
        
        f32 Font::s_globalKerningOffset = 0.0f;
        
        CS_DEFINE_NAMEDTYPE(Font);
        
        //---------------------------------------------------------------------
        //---------------------------------------------------------------------
        FontUPtr Font::Create()
        {
            return FontUPtr(new Font());
        }
		//---------------------------------------------------------------------
		//---------------------------------------------------------------------
		bool Font::IsA(Core::InterfaceIDType in_interfaceId) const
		{
			return in_interfaceId == Font::InterfaceID;
		}
		//-------------------------------------------
		//-------------------------------------------
		void Font::SetCharacterSet(const CharacterSet& in_charSet)
		{
			m_characters = in_charSet;
		}
		//-------------------------------------------
		//-------------------------------------------
		const TextureSPtr& Font::GetTexture() const
		{
            CS_ASSERT(m_spriteSheet, "Font must have a spritesheet to have a texture");
			return m_spriteSheet->GetTexture();
		}
		//-------------------------------------------
		//-------------------------------------------
		f32 Font::GetLineHeight() const
		{
			return m_lineHeight;
        }
        //-------------------------------------------
        //-------------------------------------------
        bool Font::SupportsKerning() const
        {
            return (!m_kerningLookups.empty() && !m_kerningPairs.empty());
        }
		//-------------------------------------------
		//-------------------------------------------
		void Font::SetCharacterData(const SpriteSheetSPtr& in_charData)
		{
            CS_ASSERT(m_characters.size() > 0, "Font: Cannot build characters with empty character set");
            
            m_characterInfos.clear();
			m_spriteSheet = in_charData;
			
            const f32 spriteSheetWidth = in_charData->GetSpriteSheetWidth();
            const f32 spriteSheetHeight = in_charData->GetSpriteSheetHeight();

			for (u32 i=0; i<m_characters.length(); ++i)
            {
				CharacterInfo info;
				
				SpriteSheet::Frame frame = in_charData->GetSpriteFrameByID(i);
				
				in_charData->GetUVsForFrame(i, info.m_UVs);
								
				info.m_UVs.vOrigin.x = (f32)(frame.U - 0.5f) / spriteSheetWidth;
				info.m_UVs.vOrigin.y = (f32)(frame.V - 0.5f) / spriteSheetHeight;
				info.m_UVs.vSize.x = (f32)(frame.Width + 1.0f) / spriteSheetWidth;
				info.m_UVs.vSize.y = (f32)(frame.Height + 1.0f) / spriteSheetHeight;
				
				info.m_size.x = frame.Width;
                info.m_size.y = frame.Height;
                
				info.m_offset.x = info.m_size.x * 0.5f;
				info.m_offset.y = frame.OffsetY;
                
                m_lineHeight = std::max((f32)frame.OriginalHeight, m_lineHeight);

                Core::UTF8String::Char utf8Char = m_characters[i];
				m_characterInfos.insert(std::make_pair(utf8Char, info));
			}
		
			
			//Just assign the width of a whitespaces based on the similar space character in the
			//font. This means it will scale relative to the font
            CharacterInfo info;
            if(TryGetCharacterInfo(k_similarSpaceCharacter, info) == false)
            {
                CS_LOG_ERROR("Cannot find similar space character in font: " + GetFilePath());
                info.m_size.x = 1.0f;
                info.m_offset = Core::Vector2::ZERO;
                info.m_UVs = Core::Rectangle();
            }
            
            //Space
            info.m_size.y = 0.0f;
            m_characterInfos.insert(std::make_pair(k_spaceCharacter, info));
            
            //Non-breaking space
            m_characterInfos.insert(std::make_pair(k_nbspCharacter, info));
            
            //Tab
            info.m_size.x *= k_spacesPerTab;
            m_characterInfos.insert(std::make_pair(k_tabCharacter, info));
            
            //Return
            m_characterInfos.insert(std::make_pair(k_returnCharacter, CharacterInfo()));
        }
		//-------------------------------------------
		//-------------------------------------------
		bool Font::TryGetCharacterInfo(Core::UTF8String::Char in_char, CharacterInfo& out_info) const
		{
			auto itCharEntry = m_characterInfos.find(in_char);
			
			if (itCharEntry != m_characterInfos.end())
            {
				out_info = itCharEntry->second;
				return true;
			}
			
			return false;
		}
        //-------------------------------------------
        //-------------------------------------------
        f32 Font::GetKerningBetweenCharacters(Core::UTF8String::Char in_char1, Core::UTF8String::Char in_char2) const
        {
            const KernLookup* pLookup = &(*std::lower_bound(m_kerningLookups.begin(), m_kerningLookups.end(), KernLookup(in_char1, 0)));
			if(nullptr == pLookup || pLookup->m_character != in_char1)
            {
				return s_globalKerningOffset;
            }
			
            u32 udwStart = pLookup->m_start;
			u32 udwEnd = udwStart + pLookup->m_length;
			const KernPair* pPair = &(*std::lower_bound(m_kerningPairs.begin() + udwStart, m_kerningPairs.begin() + udwEnd, KernPair(in_char2, 0.0f)));
			if(nullptr == pPair || pPair->m_character != in_char2)
            {
				return s_globalKerningOffset;
			}

            return pPair->m_spacing + s_globalKerningOffset;
        }
        //-------------------------------------------
        //-------------------------------------------
        void Font::SetKerningInfo(const std::vector<KernLookup>& in_lookups, const std::vector<KernPair>& in_pairs)
        {
            m_kerningLookups = in_lookups;
            m_kerningPairs = in_pairs;
        }
        //-------------------------------------------
        //-------------------------------------------
        void Font::SetGlobalKerningOffset(f32 in_offset)
        {
            s_globalKerningOffset = in_offset;
        }
	}
}
