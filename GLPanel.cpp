#include "GLPanel.h"

#include "Corrade/Utility/Debug.h"
#include "Corrade/Utility/DebugStl.h"



using namespace Corrade;


void GLPanel::ConnectEvents()
{
    // OnPaint event handler
    Bind(wxEVT_PAINT,       &GLPanel::OnPaint,          this);
    Bind(wxEVT_SIZE,        &GLPanel::OnResized,        this);
    Bind(wxEVT_KEY_DOWN,    &GLPanel::OnKeyDown,        this);
    Bind(wxEVT_MOTION,      &GLPanel::OnMouseMove,      this);
    Bind(wxEVT_LEFT_DOWN,   &GLPanel::OnMouseLeftDown,  this);
    Bind(wxEVT_LEFT_UP,     &GLPanel::OnMouseLeftUp,    this);
    Bind(wxEVT_MOUSEWHEEL,  &GLPanel::OnMouseScroll,    this);
}

// as the wxSmith generate such attributes which does not
// contains a WX_GL_CORE_PROFILE
// we can use a complete new GLCanvasAttributes_1
// in this cpp file, and pass it as the arguments to wxGLCanvas'
// constructor
//	int GLCanvasAttributes_1[] = {
//		WX_GL_RGBA,
//		WX_GL_DOUBLEBUFFER,
//		WX_GL_DEPTH_SIZE,      16,
//		WX_GL_STENCIL_SIZE,    0,
//		0, 0 };
int * hackAddCoreProfile(int * dispAttrs)
{
    static
    int GLCanvasAttributes_1[] =
    {
        WX_GL_CORE_PROFILE,
        WX_GL_RGBA,
        WX_GL_DOUBLEBUFFER,
        WX_GL_DEPTH_SIZE,      16,
        WX_GL_STENCIL_SIZE,    0,
        0, 0
    };
    return GLCanvasAttributes_1;
}

GLPanel::GLPanel(wxWindow *  parent,
                 wxWindowID id,
                 int*   dispAttrs,
                 const wxPoint &pos,
                 const wxSize &size,
                 long style,
                 const wxString &name,
                 const wxPalette &palette)
    : wxGLCanvas(parent, id, hackAddCoreProfile(dispAttrs), wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE),
      _glContext{NoCreate}
{
    InitContext();
    ConnectEvents();
    GLInitial();
}

GLPanel::GLPanel(wxFrame *parent, int *args)
    : wxGLCanvas(parent, wxID_ANY, hackAddCoreProfile(args), wxDefaultPosition, wxDefaultSize, wxFULL_REPAINT_ON_RESIZE),
      _glContext{NoCreate}
{
    InitContext();
    ConnectEvents();
    GLInitial();
}

GLPanel::~GLPanel() = default;


void GLPanel::InitContext()
{
#if wxCHECK_VERSION(3,1,0)
    // Explicitly create a new rendering context instance for this canvas.
    wxGLContextAttrs ctxAttrs;
    ctxAttrs.PlatformDefaults().CoreProfile().OGLVersion(3, 2).EndList();
    m_context = std::make_unique<wxGLContext>(this, nullptr, &ctxAttrs);
#else
    m_context = std::make_unique<wxGLContext>(this);
#endif

    SetCurrent(*m_context);

    // important! should create _glContext after creating the m_context.
    _glContext.create();
}


void GLPanel::GLInitial()
{
    using namespace Math::Literals;

    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);
    GL::Renderer::enable(GL::Renderer::Feature::FaceCulling);

    int w = GetSize().GetWidth();
    int h = GetSize().GetHeight();

    /* Setup a cube with vertex ID and wireframe visualized */
    {
        const Trade::MeshData cube = Primitives::cubeSolid();
        _mesh = MeshTools::compile(cube);

        const auto map = DebugTools::ColorMap::turbo();
        const Vector2i size{Int(map.size()), 1};
        _colormap = GL::Texture2D{};
        _colormap
            .setMinificationFilter(SamplerFilter::Linear)
            .setMagnificationFilter(SamplerFilter::Linear)
            .setWrapping(SamplerWrapping::ClampToEdge)
            .setStorage(1, GL::TextureFormat::RGB8, size)
            .setSubImage(0, {}, ImageView2D{PixelFormat::RGB8Unorm, size, map});

        _shader = Shaders::MeshVisualizer3D{
            Shaders::MeshVisualizer3D::Flag::Wireframe|
            Shaders::MeshVisualizer3D::Flag::VertexId};
        _shader
            .setViewportSize(Vector2{w,h})
            .setColor(0xffffff_rgbf)
            .setWireframeColor(0xffffff_rgbf)
            .setWireframeWidth(2.0f)
            .setColorMapTransformation(0.0f, 1.0f/cube.vertexCount())
            .bindColorMapTexture(_colormap);

        auto object = new Object3D{&_scene};
        (*object)
            .rotateY(40.0_degf)
            .rotateX(-30.0_degf)
            ;
        new VisualizationDrawable{*object, _shader, _mesh, _drawables};
    }

    /* Set up the camera */
    {
        /* Setup the arcball after the camera objects */
        int w = GetSize().GetWidth();
        int h = GetSize().GetHeight();

        const Vector3 eye = Vector3::zAxis(-10.0f);
        const Vector3 center{};
        const Vector3 up = Vector3::yAxis();
        _arcballCamera.emplace(_scene, eye, center, up, 45.0_degf,
            Vector2i{w,h}, Vector2i{w,h});
    }
}

void GLPanel::OnResized(wxSizeEvent &event)
{
    int w = GetSize().GetWidth();
    int h = GetSize().GetHeight();

    //GL::defaultFramebuffer.setViewport({{}, Vector2{w,h}});
    glViewport(0, 0, GetSize().GetWidth(), GetSize().GetHeight());

    _arcballCamera->reshape(Vector2i{w,h}, Vector2i{w,h});
    _shader.setViewportSize(Vector2{w,h});


    Update();
}

// OnPaint event handler
void GLPanel::OnPaint(wxPaintEvent &event)
{
    wxPaintDC(this);

    if(!IsShown())
        return;
    SetCurrent(*m_context);

    GL::defaultFramebuffer.clear(GL::FramebufferClear::Color|GL::FramebufferClear::Depth);

    /* Call arcball update in every frame. This will do nothing if the camera
       has not been changed. Otherwise, camera transformation will be
       propagated into the camera objects. */
    bool camChanged = _arcballCamera->update();
    _arcballCamera->draw(_drawables);

    SwapBuffers();
}

void GLPanel::OnKeyDown(wxKeyEvent &event)
{
    /* Reset the transformation to the original view */
    if(event.GetKeyCode() == 'L')
    {
        if(_arcballCamera->lagging() > 0.0f) {
            Debug{} << "Lagging disabled";
            _arcballCamera->setLagging(0.0f);
        } else {
            Debug{} << "Lagging enabled";
            _arcballCamera->setLagging(0.85f);
        }
        Refresh();
        return;

        /* Axis-aligned view */
    }
    else if(event.GetKeyCode() == 'R')
    {
        _arcballCamera->reset();
        Refresh();
        return;
    }

    event.Skip();
}

void GLPanel::OnMouseMove(wxMouseEvent &event)
{
    if (m_isFirstMouse)
    {
        m_lastMouse = event.GetPosition();
        m_isFirstMouse = false;
    }
    wxPoint mouse = event.GetPosition();
    float x_offset = (float)mouse.x - (float)m_lastMouse.x;
    float y_offset = (float)mouse.y - (float)m_lastMouse.y;

    bool needRefresh = false;

    Vector2 delta = Vector2{x_offset, y_offset};
    Vector2i pos(mouse.x, mouse.y);


    if(event.ButtonIsDown(wxMOUSE_BTN_LEFT))
    {
       _arcballCamera->rotate(pos);

        needRefresh = true;
    }
    else if (event.ButtonIsDown(wxMOUSE_BTN_RIGHT))
    {

        _arcballCamera->translate(pos);
        needRefresh = true;
    }

    m_lastMouse = event.GetPosition();

    if (needRefresh)
        Refresh();

    event.Skip();
}

void GLPanel::OnMouseScroll(wxMouseEvent &event)
{

    const float delta = (float)event.GetWheelRotation() / 50;

    if(Math::abs(delta) < 1.0e-2f) return;

    _arcballCamera->zoom(delta);

    Refresh();
    event.Skip();
}

void GLPanel::OnMouseLeftDown(wxMouseEvent &event)
{
    CaptureMouse();

    wxPoint mouse = event.GetPosition();
    Vector2i pos(mouse.x, mouse.y);

    _arcballCamera->initTransformation(pos);

    event.Skip();
}

void GLPanel::OnMouseLeftUp(wxMouseEvent &event)
{
    if ( HasCapture() )
        ReleaseMouse();

    event.Skip();
}

