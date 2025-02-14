#pragma once
#include "PrecompiledHeader.h"
#include "Utilities/NoCopy.h"
#include "Utilities/NativeInterface.h"
#include "Utilities/Ref.h"
#include "Rendering/Structs/FenceRef.h"
#include "Math/Types.h"

namespace PK::Core
{
    struct WindowProperties
    {
        std::string title;
        std::string iconPath;
        uint32_t width;
        uint32_t height;
        bool vsync;
        bool cursorVisible;
    
        WindowProperties(const std::string& title = "PK Window", const std::string& iconPath = std::string(), uint32_t width = 1600, uint32_t height = 900, bool vsync = true, bool cursorVisible = true) :
            title(title), iconPath(iconPath), width(width), height(height), vsync(vsync), cursorVisible(cursorVisible)
        {
        }
    };
    
    class Window : public Utilities::NoCopy, public Utilities::NativeInterface<Window>
    {
        public:
            static Utilities::Scope<Window> Create(const WindowProperties& properties);
            virtual ~Window() = default;
        
            virtual Math::uint3 GetResolution() const = 0;
            virtual Math::uint4 GetRect() const = 0;
            virtual float GetAspectRatio() const = 0;
            virtual bool IsAlive() const = 0;
            virtual bool IsMinimized() const = 0;
            virtual bool IsVSync() const = 0;
            inline static void SetConsole(bool enabled) { ::ShowWindow(::GetConsoleWindow(), enabled ? SW_SHOW : SW_HIDE); }
            
            virtual void Begin() = 0;
            virtual void End() = 0;
            virtual void SetFrameFence(const Rendering::Structs::FenceRef& fence) = 0;
            virtual void SetCursorVisible(bool value) = 0;
            virtual void SetVSync(bool enabled) = 0;
            virtual void PollEvents() const = 0;
            virtual void WaitEvents() const = 0;
            virtual void* GetNativeWindow() const = 0;

        public:
            std::function<void(int width, int height)> OnResize;
            std::function<void()> OnClose;
            std::function<void(int key, int scancode, int action, int mods)> OnKeyInput;
            std::function<void(unsigned int keycode)> OnCharInput;
            std::function<void(int button, int action, int mods)> OnMouseButtonInput;
            std::function<void(double xOffset, double yOffset)> OnScrollInput;
            std::function<void(double xPos, double yPos)> OnCursorInput;
    };
}