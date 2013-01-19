
/* === Macros === */

#define SP_TESTS_DECLARE
    SoftPixelDevice* spDevice           = 0;    \
                                                \
    video::RenderSystem* spRenderer     = 0;    \
    video::RenderContext* spContext     = 0;    \
    io::InputControl* spControl         = 0;    \
    scene::SceneManager* spSceneMngr    = 0;    \
                                                \
    scene::SceneGraph* spScene          = 0;    \
                                                \
    scene::Camera* Cam                  = 0;    \
    scene::Light* Lit                   = 0;

#define SP_TESTS_INIT_EX2(RS, RES, TITLE, FS, FLAGS)        \
    spDevice = createGraphicsDevice(                        \
        RS, RES, 32, "Tests: " # TITLE, FS, FLAGS           \
    );                                                      \
                                                            \
    spRenderer  = spDevice->getRenderSystem();              \
    spContext   = spDevice->getRenderContext();             \
    spControl   = spDevice->getInputControl();              \
    spSceneMngr = spDevice->getSceneManager();              \
                                                            \
    spScene     = spDevice->createSceneGraph();             \
                                                            \
    spContext->setWindowTitle(                              \
        spContext->getWindowTitle() +                       \
        " [ " + spRenderer->getVersion() + " ]"             \
    );                                                      \
                                                            \
    Cam = spScene->createCamera();                          \
    Cam->setRange(0.1f, 1000.0f);                           \
                                                            \
    Lit = spScene->createLight();                           \
    Lit->setRotation(dim::vector3df(25, 25, 0));            \
    spScene->setLighting();

#define SP_TESTS_INIT_EX(RS, RES, TITLE, FS) \
    SP_TESTS_INIT_EX2(RS, RES, TITLE, FS, DEVICEFLAG_HQ)

#define SP_TESTS_INIT(n) \
    SP_TESTS_INIT_EX(video::RENDERER_OPENGL, dim::size2di(800, 600), n, false)

#define SP_TESTS_MAIN_BEGIN                                                     \
    while (spDevice->updateEvents() && !spControl->keyDown(io::KEY_ESCAPE))     \
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

