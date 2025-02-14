#include "PrecompiledHeader.h"
#include "Application.h"
#include "Core/Services/Log.h"
#include "Core/Services/StringHashID.h"
#include "Core/Services/Input.h"
#include "Core/Services/Time.h"
#include "Core/Services/AssetDatabase.h"
#include "Core/Services/Sequencer.h"
#include "Core/ApplicationConfig.h"
#include "Core/CommandConfig.h"
#include "Core/UpdateStep.h"
#include "ECS/EntityDatabase.h"
#include "ECS/Contextual/Engines/EngineCommandInput.h"
#include "ECS/Contextual/Engines/EngineEditorCamera.h"
#include "ECS/Contextual/Engines/EngineBuildAccelerationStructure.h"
#include "ECS/Contextual/Engines/EngineUpdateTransforms.h"
#include "ECS/Contextual/Engines/EnginePKAssetBuilder.h"
#include "ECS/Contextual/Engines/EngineCull.h"
#include "ECS/Contextual/Engines/EngineDebug.h"
#include "ECS/Contextual/Engines/EngineScreenshot.h"
#include "ECS/Contextual/Tokens/TimeToken.h"
#include "Rendering/RenderPipeline.h"
#include "Rendering/HashCache.h"

namespace PK::Core
{
    using namespace Utilities;
    using namespace Rendering;
    using namespace Rendering::Structs;
    using namespace Rendering::Objects;
    using namespace Services;

    Application* Application::s_Instance = nullptr;

    Application::Application(ApplicationArguments arguments, const std::string& name)
    {
        PK_THROW_ASSERT(!s_Instance, "Application already exists!");
        s_Instance = this;

        uint32_t logfilter = 0u;
        logfilter |= Debug::PK_LOG_LVL_INFO;
        logfilter |= Debug::PK_LOG_LVL_ERROR;
        logfilter |= Debug::PK_LOG_LVL_WARNING;
        logfilter |= Debug::PK_LOG_LVL_VERBOSE;

        m_services = CreateScope<ServiceRegister>();
        m_services->Create<Debug::Logger>(logfilter);
        m_services->Create<StringHashID>();
        m_services->Create<HashCache>();

        auto entityDb = m_services->Create<PK::ECS::EntityDatabase>();
        auto sequencer = m_services->Create<Sequencer>();
        auto assetDatabase = m_services->Create<AssetDatabase>(sequencer);

        assetDatabase->LoadDirectory<ApplicationConfig>("res/configs/");
        assetDatabase->LoadDirectory<CommandConfig>("res/configs/");
        auto config = assetDatabase->Find<ApplicationConfig>("Active");
        auto commandConfig = assetDatabase->Find<CommandConfig>("Active");

        auto time = m_services->Create<Time>(sequencer, config->TimeScale);
        auto input = m_services->Create<Input>(sequencer);

        auto workingDirectory = std::filesystem::path(arguments.args[0]).remove_filename().string();
        m_graphicsDriver = GraphicsDriver::Create(workingDirectory, APIType::Vulkan);

        m_window = Window::Create(WindowProperties(name + m_graphicsDriver->GetDriverHeader(),
            config->FileWindowIcon,
            config->InitialWidth,
            config->InitialHeight,
            config->EnableVsync,
            config->EnableCursor));

        Window::SetConsole(config->EnableConsole);
        m_window->OnKeyInput = PK_BIND_FUNCTION(input, OnKeyInput);
        m_window->OnScrollInput = PK_BIND_FUNCTION(input, OnScrollInput);
        m_window->OnMouseButtonInput = PK_BIND_FUNCTION(input, OnMouseButtonInput);
        m_window->OnClose = PK_BIND_FUNCTION(this, Application::Close);

        assetDatabase->LoadDirectory<Shader>("res/shaders/");

        auto engineEditorCamera = m_services->Create<ECS::Engines::EngineEditorCamera>(sequencer, time, config);
        auto engineUpdateTransforms = m_services->Create<ECS::Engines::EngineUpdateTransforms>(entityDb);
        auto renderPipeline = m_services->Create<RenderPipeline>(assetDatabase, entityDb, sequencer, config);
        auto engineCommands = m_services->Create<ECS::Engines::EngineCommandInput>(assetDatabase, sequencer, time, entityDb, commandConfig);
        auto engineCull = m_services->Create<ECS::Engines::EngineCull>(entityDb);
        auto engineBuildAccelerationStructure = m_services->Create<ECS::Engines::EngineBuildAccelerationStructure>(entityDb);
        auto engineDebug = m_services->Create<ECS::Engines::EngineDebug>(assetDatabase, entityDb, config);
        auto enginePKAssetBuilder = m_services->Create<ECS::Engines::EnginePKAssetBuilder>(arguments);
        auto engineScreenshot = m_services->Create<ECS::Engines::EngineScreenshot>();

        sequencer->SetSteps(
            {
                {
                    sequencer->GetRoot(),
                    {
                        { (int)UpdateStep::OpenFrame,		{ Step::Simple(time) }},
                        { (int)UpdateStep::UpdateInput,		{ Step::Conditional<Window>(input) } },
                        { (int)UpdateStep::UpdateEngines,   { Step::Simple(engineUpdateTransforms) } },
                        { (int)UpdateStep::Render,			{ Step::Conditional<Window>(renderPipeline), Step::Token<Window>(engineScreenshot) } },
                        { (int)UpdateStep::CloseFrame,		{ Step::Conditional<Window>(input), Step::Simple(time) }},
                    }
                },
                {
                    time,
                    {
                        Step::Token<PK::ECS::Tokens::TimeToken>(renderPipeline)
                    }
                },
                {
                    input,
                    {
                        Step::Token<Input>(engineCommands),
                        Step::Token<Input>(engineEditorCamera),
                    }
                },
                {
                    engineCommands,
                    {
                        Step::Token<TokenConsoleCommand>(engineEditorCamera),
                        Step::Token<TokenConsoleCommand>(enginePKAssetBuilder),
                        Step::Token<TokenConsoleCommand>(engineScreenshot),
                        //PK_STEP_T(gizmoRenderer, ConsoleCommandToken),
                    }
                },
                {
                    engineEditorCamera,
                    {
                        Step::Token<PK::ECS::Tokens::ViewProjectionUpdateToken>(renderPipeline)
                    }
                },
                {
                    renderPipeline,
                    {
                        Step::Token<PK::ECS::Tokens::TokenCullFrustum>(engineCull),
                        Step::Token<PK::ECS::Tokens::TokenCullCascades>(engineCull),
                        Step::Token<PK::ECS::Tokens::TokenCullCubeFaces>(engineCull),
                        Step::Token<PK::ECS::Tokens::AccelerationStructureBuildToken>(engineBuildAccelerationStructure)
                    }
                },
                {
                    assetDatabase,
                    {
                        Step::Token<AssetImportToken<ApplicationConfig>>(renderPipeline)
                    }
                }
            });

        PK_LOG_HEADER("----------INITIALIZATION COMPLETE----------");
    }

    Application::~Application()
    {
        GetService<Services::Sequencer>()->Release();
        GetService<AssetDatabase>()->Unload();
        m_services->Clear();
        m_window = nullptr;
        m_graphicsDriver = nullptr;
    }

    void Application::Execute()
    {
        auto sequencer = GetService<Services::Sequencer>();

        while (m_window->IsAlive() && m_Running)
        {
            m_window->PollEvents();

            if (m_window->IsMinimized())
            {
                m_window->WaitEvents();
                continue;
            }

            sequencer->Next((int)UpdateStep::OpenFrame);

            m_window->Begin();

            sequencer->Next<PK::Core::Window>(m_window.get(), (int)UpdateStep::OpenFrame);
            sequencer->Next<PK::Core::Window>(m_window.get(), (int)UpdateStep::UpdateInput);
            sequencer->Next((int)UpdateStep::UpdateEngines);
            sequencer->Next<PK::Core::Window>(m_window.get(), (int)UpdateStep::Render);
            sequencer->Next<PK::Core::Window>(m_window.get(), (int)UpdateStep::CloseFrame);

            m_window->End();

            sequencer->Next((int)UpdateStep::CloseFrame);

            GraphicsAPI::GC();
        }
    }

    void Application::Close()
    {
        m_Running = false;
    }
}