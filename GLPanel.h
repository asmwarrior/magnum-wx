#ifndef _GL_PANEL_H
#define _GL_PANEL_H

#include "wx/wx.h"



#include <Magnum/Platform/GLContext.h>


#include <Magnum/Image.h>
#include <Magnum/GL/Buffer.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/PixelFormat.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Math/FunctionsBatch.h>
#include <Magnum/MeshTools/Compile.h>
//#include <Magnum/Platform/Sdl2Application.h>
#include <Magnum/Primitives/Grid.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/SceneGraph/MatrixTransformation3D.h>
#include <Magnum/SceneGraph/Object.h>
#include <Magnum/SceneGraph/Scene.h>
#include <Magnum/Shaders/Flat.h>
#include <Magnum/Shaders/VertexColor.h>
#include <Magnum/Trade/MeshData.h>
#include <Magnum/Shaders/MeshVisualizer.h>

#include <Corrade/Containers/Optional.h>
#include <Magnum/DebugTools/ColorMap.h>
#include <Magnum/ImageView.h>
#include <Magnum/PixelFormat.h>
#include <Magnum/GL/DefaultFramebuffer.h>
#include <Magnum/GL/Mesh.h>
#include <Magnum/GL/Renderer.h>
#include <Magnum/GL/Texture.h>
#include <Magnum/GL/TextureFormat.h>
#include <Magnum/Math/Color.h>
#include <Magnum/Math/Matrix4.h>
#include <Magnum/MeshTools/Compile.h>
//#include <Magnum/Platform/Sdl2Application.h>
#include <Magnum/Primitives/Cube.h>
#include <Magnum/SceneGraph/Camera.h>
#include <Magnum/SceneGraph/Drawable.h>
#include <Magnum/SceneGraph/MatrixTransformation3D.h>
#include <Magnum/SceneGraph/Object.h>
#include <Magnum/SceneGraph/Scene.h>
#include <Magnum/Shaders/MeshVisualizer.h>
#include <Magnum/Trade/MeshData.h>


#include <Magnum/Primitives/Cube.h>
#include <Magnum/Shaders/Phong.h>


#include <Magnum/Math/Matrix4.h>
#include <Magnum/MeshTools/Interleave.h>
#include <Magnum/MeshTools/CompressIndices.h>
//#include <Magnum/Platform/Sdl2Application.h>

using namespace Magnum;


#include <wx/app.h>
#include <wx/frame.h>
#include <wx/glcanvas.h>
#include <wx/sizer.h>
#include <wx/version.h>

#include "ArcBall.h"
#include "ArcBallCamera.h"


using Object3D = SceneGraph::Object<SceneGraph::MatrixTransformation3D>;
using Scene3D = SceneGraph::Scene<SceneGraph::MatrixTransformation3D>;
using namespace Math::Literals;

using namespace Magnum::Examples;

class GLPanel : public wxGLCanvas
{
public:

    GLPanel (wxWindow *  parent,
                wxWindowID id = wxID_ANY,
                int*   dispAttrs = nullptr,
                const wxPoint &pos=wxDefaultPosition,
                const wxSize &size=wxDefaultSize,
                long style=0,
                const wxString &name=wxGLCanvasName,
                const wxPalette &palette=wxNullPalette);

    GLPanel(wxFrame *parent, int *args);
    ~GLPanel() override;


    Float depthAt(const Vector2i& windowPosition);
    Vector3 unproject(const Vector2i& windowPosition, Float depth) const;


    void InitContext(); // the hack here, should be created before other members(mesh, shader)

    void ConnectEvents();

    void GLInitial();


    void OnResized(wxSizeEvent &event);
    void OnPaint(wxPaintEvent &event);
    void OnKeyDown(wxKeyEvent &event);
    void OnMouseLeftDown(wxMouseEvent &event);
    void OnMouseLeftUp(wxMouseEvent &event);
    void OnMouseMove(wxMouseEvent &event);
    void OnMouseScroll(wxMouseEvent &event);

private:

    bool m_isFirstMouse = false;
    wxPoint m_lastMouse;

    std::unique_ptr<wxGLContext> m_context;

    Platform::GLContext _glContext;

    // create those objects with {NoCreate}
    // see description here: https://doc.magnum.graphics/magnum/opengl-wrapping.html#opengl-wrapping-instances-nocreate
    Scene3D _scene;
    SceneGraph::DrawableGroup3D _drawables;
    GL::Mesh _mesh{NoCreate};
    Containers::Optional<ArcBallCamera> _arcballCamera;

    /* Stuff for visualizing the cube */
    Shaders::MeshVisualizer3D _shader{NoCreate};
    GL::Texture2D _colormap{NoCreate};
};


class VertexColorDrawable: public SceneGraph::Drawable3D {
    public:
        explicit VertexColorDrawable(Object3D& object, Shaders::VertexColor3D& shader, GL::Mesh& mesh, SceneGraph::DrawableGroup3D& drawables): SceneGraph::Drawable3D{object, &drawables}, _shader(shader), _mesh(mesh) {}

        void draw(const Matrix4& transformation, SceneGraph::Camera3D& camera) {
            _shader
                .setTransformationProjectionMatrix(camera.projectionMatrix()*transformation)
                .draw(_mesh);
        }

    private:
        Shaders::VertexColor3D& _shader;
        GL::Mesh& _mesh;
};

class FlatDrawable: public SceneGraph::Drawable3D {
    public:
        explicit FlatDrawable(Object3D& object, Shaders::Flat3D& shader, GL::Mesh& mesh, SceneGraph::DrawableGroup3D& drawables): SceneGraph::Drawable3D{object, &drawables}, _shader(shader), _mesh(mesh) {}

        void draw(const Matrix4& transformation, SceneGraph::Camera3D& camera) {
            _shader
                .setColor(0x747474_rgbf)
                .setTransformationProjectionMatrix(camera.projectionMatrix()*transformation)
                .draw(_mesh);
        }

    private:
        Shaders::Flat3D& _shader;
        GL::Mesh& _mesh;
};

class VisualizationDrawable: public SceneGraph::Drawable3D {
    public:
        explicit VisualizationDrawable(Object3D& object,
            Shaders::MeshVisualizer3D& shader, GL::Mesh& mesh,
            SceneGraph::DrawableGroup3D& drawables):
                SceneGraph::Drawable3D{object, &drawables}, _shader(shader),
                _mesh(mesh) {}

        void draw(const Matrix4& transformation, SceneGraph::Camera3D& camera) {
            _shader
                .setTransformationMatrix(transformation)
                .setProjectionMatrix(camera.projectionMatrix())
                .draw(_mesh);
        }

    private:
        Shaders::MeshVisualizer3D& _shader;
        GL::Mesh& _mesh;
};

#endif
