//
//  AppConfig.cpp
//  Chilli Source
//  Created by Ian Copland on 02/06/2014.
//
//  The MIT License (MIT)
//
//  Copyright (c) 2014 Tag Games Limited
//
//  Permission is hereby granted, free of charge, to any person obtaining a copy
//  of this software and associated documentation files (the "Software"), to deal
//  in the Software without restriction, including without limitation the rights
//  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
//  copies of the Software, and to permit persons to whom the Software is
//  furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included in
//  all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
//  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//  THE SOFTWARE.
//

#include <ChilliSource/Core/Base/AppConfig.h>

#include <ChilliSource/Core/Base/Utils.h>
#include <ChilliSource/Core/File/StorageLocation.h>
#include <ChilliSource/Core/JSON/json.h>
#include <ChilliSource/Core/String/StringParser.h>
#include <ChilliSource/Core/String/StringUtils.h>
#include <ChilliSource/Rendering/Base/SurfaceFormat.h>

namespace ChilliSource
{
    namespace Core
    {
        namespace
        {
            const std::string k_defaultDisplayableName = "Chilli Source App";
            const u32 k_defaultPreferredFPS = 30;
            const std::string k_defaultPreferredSurfaceFormat = "rgb545_depth24";
        }
        
        CS_DEFINE_NAMEDTYPE(AppConfig);
        
        const std::string AppConfig::k_filePath = "App.config";
        
        //---------------------------------------------------------
        //---------------------------------------------------------
        AppConfigUPtr AppConfig::Create()
        {
            return AppConfigUPtr(new AppConfig());
        }
        //---------------------------------------------------------
        //---------------------------------------------------------
        AppConfig::AppConfig()
        : m_preferredFPS(k_defaultPreferredFPS), m_displayableName(k_defaultDisplayableName), m_preferredSurfaceFormat(Rendering::SurfaceFormat::k_rgb545_depth24)
        {
        }
        //---------------------------------------------------------
        //--------------------------------------------------------
        bool AppConfig::IsA(InterfaceIDType in_interfaceId) const
        {
            return (AppConfig::InterfaceID == in_interfaceId);
        }
        //---------------------------------------------------------
        //--------------------------------------------------------
        const std::string& AppConfig::GetDisplayableName() const
        {
            return m_displayableName;
        }
        //---------------------------------------------------------
        //--------------------------------------------------------
        u32 AppConfig::GetPreferredFPS() const
        {
            return m_preferredFPS;
        }
        //---------------------------------------------------------
        //--------------------------------------------------------
        Rendering::SurfaceFormat AppConfig::GetPreferredSurfaceFormat() const
        {
            return m_preferredSurfaceFormat;
        }
        //---------------------------------------------------------
        //---------------------------------------------------------
        void AppConfig::Load()
        {
            Json::Value root;
            if(Utils::ReadJson(StorageLocation::k_package, k_filePath, &root) == true)
            {
                m_displayableName = root.get("DisplayableName", k_defaultDisplayableName).asString();
                m_preferredFPS = root.get("PreferredFPS", k_defaultPreferredFPS).asUInt();
                m_preferredSurfaceFormat = ParseSurfaceFormat(root.get("PreferredSurfaceFormat", k_defaultPreferredSurfaceFormat).asString());
            }
        }
    }
}
