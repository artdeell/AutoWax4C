#ifndef SKY_MODLOADER_ICONLOADER_H
#define SKY_MODLOADER_ICONLOADER_H
#include "../imgui/imgui.h"

#define IL_NO_TEXTURE (ImTextureID)-1

class SkyImage {
public:
    ImTextureID textureId;
    ImVec2 size;
};

class IconLoader {
public:
    /**
    * Gets an image under the name "name" from Data/Images/bin/ETC2/<name>.ktx, ready to be drawn using ImGui::Image or ImGui::ImageButton if it was loaded successfully
    * Returns: a SkyImage object
    * If the image fails to load for some reason, the textureId would be equal to IL_NO_TEXTURE, and must not be drawn by your code.
    */
    static SkyImage& getImage(const std::string& name);
    /**
    * Draws an icon, loaded by its name from UiPackedAtlas.lua, with the specified name and size in pixels
    */
    static void icon(const std::string& name, const float& size, const ImVec4& color = ImVec4(1,1,1,1));
    /**
    * Creates a button with an icon, loaded by its name from UiPackedAtlas.lua, with the specified name and size in pixels
    */
    static bool iconButton(const std::string& name, const float& size, const ImVec4& color = ImVec4(1,1,1,1));
};


#endif //SKY_MODLOADER_ICONLOADER_H
