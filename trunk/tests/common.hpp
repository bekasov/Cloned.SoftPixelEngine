
/* === Macros === */

#define SP_TESTS_DECLARE
    SoftPixelDevice* spDevice = 0;          \
                                            \
    video::RenderSystem* spRenderer = 0;    \
    video::RenderContext* spContext = 0;    \
    io::InputControl* spControl     = 0;    \
                                            \
    scene::SceneGraph* spScene      = 0;    \
                                            \
    scene::Camera* Cam              = 0;    \
    scene::Light* Lit               = 0;

#define SP_TESTS_INIT(n)                                                        \
    spDevice = createGraphicsDevice(                                            \
        video::RENDERER_OPENGL, dim::size2di(800, 600), 32,                     \
        "Tests: " # n, false, DEVICEFLAG_HQ                                     \
    );                                                                          \
                                                                                \
    spRenderer = spDevice->getRenderSystem();                                   \
    spContext = spDevice->getRenderContext();                                   \
    spControl     = spDevice->getInputControl();                                \
                                                                                \
    spScene      = spDevice->createSceneGraph();                                \
                                                                                \
    spContext->setWindowTitle(                                                  \
        spContext->getWindowTitle() + " [ " + spRenderer->getVersion() + " ]"   \
    );                                                                          \
                                                                                \
    Cam = spScene->createCamera();                                              \
    Cam->setRange(0.1f, 1000.0f);                                               \
                                                                                \
    Lit = spScene->createLight();                                               \
    Lit->setRotation(dim::vector3df(25, 25, 0));                                \
    spScene->setLighting();

#define SP_TESTS_MAIN_BEGIN                                                     \
    while (spDevice->updateEvent() && !spControl->keyDown(io::KEY_ESCAPE))      \
    {                                                                           \
        spRenderer->clearBuffers();

#define SP_TESTS_MAIN_END                                                       \
        spScene->renderScene();                                                 \
                                                                                \
        spContext->flipBuffers();                                               \
    }                                                                           \
                                                                                \
    deleteDevice();                                                             \
                                                                                \
    return 0;


/* === Static functions === */

static sp::s32 Fatal(const sp::io::stringc &ErrorStr)
{
    sp::io::Log::error(ErrorStr, sp::io::LOG_MSGBOX);
    return 0;
}
