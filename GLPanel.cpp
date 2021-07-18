#include "GLPanel.h"

#include "Corrade/Utility/Debug.h"
#include "Corrade/Utility/DebugStl.h"



using namespace Corrade;


Float GLPanel::depthAt(const Vector2i& windowPosition)
{
    /* First scale the position from being relative to window size to being
       relative to framebuffer size as those two can be different on HiDPI
       systems */

    int w = GetSize().GetWidth();
    int h = GetSize().GetHeight();
    //const Vector2i position = windowPosition*Vector2{framebufferSize()}/Vector2{windowSize()};
    const Vector2i position = windowPosition*Vector2{w,h}/Vector2{w,h};

    const Vector2i fbPosition{position.x(), GL::defaultFramebuffer.viewport().sizeY() - position.y() - 1};

    GL::defaultFramebuffer.mapForRead(GL::DefaultFramebuffer::ReadAttachment::Front);
    Image2D data = GL::defaultFramebuffer.read(Range2Di::fromSize(fbPosition, Vector2i{1}).padded(Vector2i{2}),
                                               {GL::PixelFormat::DepthComponent, GL::PixelType::Float});

    /* TODO: change to just Math::min<Float>(data.pixels<Float>() when the
       batch functions in Math can handle 2D views */
    return Math::min<Float>(data.pixels<Float>().asContiguous());
}

Vector3 GLPanel::unproject(const Vector2i& windowPosition, Float depth) const
{
    /* We have to take window size, not framebuffer size, since the position is
       in window coordinates and the two can be different on HiDPI systems */

    int w = GetSize().GetWidth();
    int h = GetSize().GetHeight();

    //const Vector2i viewSize = windowSize();

    const Vector2i viewSize{w,h}; // = windowSize();
    const Vector2i viewPosition{windowPosition.x(), viewSize.y() - windowPosition.y() - 1};
    const Vector3 in{2*Vector2{viewPosition}/Vector2{viewSize} - Vector2{1.0f}, depth*2.0f - 1.0f};

    /*
        Use the following to get global coordinates instead of camera-relative:

        (_cameraObject->absoluteTransformationMatrix()*_camera->projectionMatrix().inverted()).transformPoint(in)
    */
    return _camera->projectionMatrix().inverted().transformPoint(in);
}


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

    /* Shaders, renderer setup */
    _vertexColorShader = Shaders::VertexColor3D{};
    _flatShader = Shaders::Flat3D{};
    GL::Renderer::enable(GL::Renderer::Feature::DepthTest);

    /* Triangle data */
    const struct
    {
        Vector3 pos;
        Color3 color;
    } data[] {
        {{-1.0f, -1.0f, 0.0f}, 0xff0000_rgbf},
        {{ 1.0f, -1.0f, 0.0f}, 0x00ff00_rgbf},
        {{ 0.0f,  1.0f, 0.0f}, 0x0000ff_rgbf}
    };

    /* Triangle mesh */
    GL::Buffer buffer;
    buffer.setData(data);
    _mesh = GL::Mesh{};
    _mesh.setCount(3)
         .addVertexBuffer(std::move(buffer), 0,
                          Shaders::VertexColor3D::Position{},
                          Shaders::VertexColor3D::Color3{});

    /* Triangle object */
    auto triangle = new Object3D{&_scene};
    new VertexColorDrawable{*triangle, _vertexColorShader, _mesh, _drawables};

    /* Grid */
    _grid = MeshTools::compile(Primitives::grid3DWireframe({15, 15}));
    auto grid = new Object3D{&_scene};
    (*grid)
        .rotateX(90.0_degf)
        .scale(Vector3{8.0f});
    new FlatDrawable{*grid, _flatShader, _grid, _drawables};

    /* Set up the camera */
    _cameraObject = new Object3D{&_scene};
    (*_cameraObject)
        .translate(Vector3::zAxis(5.0f))
        .rotateX(-15.0_degf)
        .rotateY(30.0_degf);
    _camera = new SceneGraph::Camera3D{*_cameraObject};

    int w = GetSize().GetWidth();
    int h = GetSize().GetHeight();

    _camera->setProjectionMatrix(Matrix4::perspectiveProjection(45.0_degf, Vector2{w,h}.aspectRatio(), 0.01f, 100.0f));

    /* Initialize initial depth to the value at scene center */
    _lastDepth = ((_camera->projectionMatrix()*_camera->cameraMatrix()).transformPoint({}).z() + 1.0f) * 0.5f;

    Utility::Debug{} << "_camera->projectionMatrix()" << _camera->projectionMatrix();

    Utility::Debug{} << "_camera->cameraMatrix()" << _camera->cameraMatrix();

    Utility::Debug{} << "_lastDepth" << _lastDepth;
}

void GLPanel::OnResized(wxSizeEvent &event)
{
    glViewport(0, 0, GetSize().GetWidth(), GetSize().GetHeight());
    Update();
}

// OnPaint event handler
void GLPanel::OnPaint(wxPaintEvent &event)
{
    wxPaintDC(this);

    if(!IsShown())
        return;
    SetCurrent(*m_context);

    GL::defaultFramebuffer.clear(
        GL::FramebufferClear::Color|GL::FramebufferClear::Depth);

    _camera->draw(_drawables);

    SwapBuffers();
}

void GLPanel::OnKeyDown(wxKeyEvent &event)
{
    /* Reset the transformation to the original view */
    if(event.GetKeyCode() == '0')
    {
        (*_cameraObject)
            .resetTransformation()
            .translate(Vector3::zAxis(5.0f))
            .rotateX(-15.0_degf)
            .rotateY(30.0_degf);
        Refresh();
        return;

        /* Axis-aligned view */
    }
    else if(event.GetKeyCode() == '1'||
            event.GetKeyCode() == '3'||
            event.GetKeyCode() == '7')
    {
        /* Start with current camera translation with the rotation inverted */
        const Vector3 viewTranslation = _cameraObject->transformation().rotationScaling().inverted()*_cameraObject->transformation().translation();

        /* Front/back */
        const Float multiplier = event.GetKeyCode() == wxKeyCode::WXK_CONTROL ? -1.0f : 1.0f;

        Matrix4 transformation;
        if(event.GetKeyCode() == '1') /* Top/bottom */
            transformation = Matrix4::rotationX(-90.0_degf*multiplier);
        else if(event.GetKeyCode() == '3') /* Front/back */
            transformation = Matrix4::rotationY(90.0_degf - 90.0_degf*multiplier);
        else if(event.GetKeyCode() == '7') /* Right/left */
            transformation = Matrix4::rotationY(90.0_degf*multiplier);
        else CORRADE_INTERNAL_ASSERT_UNREACHABLE();

        _cameraObject->setTransformation(transformation*Matrix4::translation(viewTranslation));
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
        /* Rotate around rotation point */
        _cameraObject->transformLocal(
            Matrix4::translation(_rotationPoint)*
            Matrix4::rotationX(-0.01_radf*delta.y())*
            Matrix4::rotationY(-0.01_radf*delta.x())*
            Matrix4::translation(-_rotationPoint));

        std::cout << "_rotationPoint" << _rotationPoint.x() << "  "  << _rotationPoint.y() <<  "  " << _rotationPoint.z() << std::endl;
        std::cout << "Left mouse button drag" << std::endl;

        Utility::Debug{} << "_rotationPoint" << _rotationPoint;

        Utility::Debug{} << "delta" << delta ;

        needRefresh = true;
    }
    else if (event.ButtonIsDown(wxMOUSE_BTN_RIGHT))
    {
        const Vector3 p = unproject(pos, _lastDepth);
        _cameraObject->translateLocal(_translationPoint - p); /* is Z always 0? */
        _translationPoint = p;

        std::cout<< "Right mouse button drag" << std::endl;

        needRefresh = true;
    }

    m_lastMouse = event.GetPosition();

    if (needRefresh)
        Refresh();

    event.Skip();
}

void GLPanel::OnMouseScroll(wxMouseEvent &event)
{
    //float scroll = (float)event.GetWheelRotation() / 50;
    float scroll = (float)event.GetWheelRotation();

    Vector2i pos{event.GetX(), event.GetY()};
    const Float currentDepth = depthAt(pos);
    const Float depth = currentDepth == 1.0f ? _lastDepth : currentDepth;
    const Vector3 p = unproject(pos, depth);
    /* Update the rotation point only if we're not zooming against infinite
       depth or if the original rotation point is not yet initialized */
    if(currentDepth != 1.0f || _rotationPoint.isZero())
    {
        _rotationPoint = p;
        _lastDepth = depth;
    }

    //const Float direction = event.offset().y();
    // fixme
    const Float direction = scroll;

    if(!direction)
        return;

    /* Move towards/backwards the rotation point in cam coords */
    _cameraObject->translateLocal(_rotationPoint*direction*0.1f);

    Utility::Debug{} << "_rotationPoint" << _rotationPoint;

    Utility::Debug{} << "direction" << direction ;

    Refresh();
    event.Skip();
}

void GLPanel::OnMouseLeftDown(wxMouseEvent &event)
{
    Vector2i pos{event.GetX(), event.GetY()};

    const Float currentDepth = depthAt(pos);
    const Float depth = currentDepth == 1.0f ? _lastDepth : currentDepth;
    _translationPoint = unproject(pos, depth);

    Utility::Debug{} << "_translationPoint" << _translationPoint;

    /* Update the rotation point only if we're not zooming against infinite
       depth or if the original rotation point is not yet initialized */
    if(currentDepth != 1.0f || _rotationPoint.isZero())
    {
        _rotationPoint = _translationPoint;
        _lastDepth = depth;
    }

    Utility::Debug{} << "_rotationPoint" << _rotationPoint;

    Utility::Debug{} << "pos" << pos ;

    event.Skip();
}

void GLPanel::OnMouseLeftUp(wxMouseEvent &event)
{
    event.Skip();
}

