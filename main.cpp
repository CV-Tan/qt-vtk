// Demo: a cone rendered in a QVTKOpenGLNativeWidget, with a "auto rotate"
// checkbox (checked by default). Works with both Qt5 and Qt6.
#include "QVTKOpenGLNativeWidget.h"

#include <QApplication>
#include <QCheckBox>
#include <QSurfaceFormat>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

#include <vtkActor.h>
#include <vtkCamera.h>
#include <vtkConeSource.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkNew.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>

int main(int argc, char* argv[])
{
  // must be set before QApplication is created
  QSurfaceFormat::setDefaultFormat(QVTKOpenGLNativeWidget::defaultFormat());
  QApplication app(argc, argv);

  // --- VTK scene: a cone ---
  vtkNew<vtkConeSource> cone;
  cone->SetResolution(48);

  vtkNew<vtkPolyDataMapper> mapper;
  mapper->SetInputConnection(cone->GetOutputPort());

  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);

  vtkNew<vtkRenderer> renderer;
  renderer->AddActor(actor);
  renderer->SetBackground(0.2, 0.3, 0.4); // 渲染控件背景色
  renderer->ResetCamera();

  vtkNew<vtkGenericOpenGLRenderWindow> renderWindow;
  renderWindow->AddRenderer(renderer);

  // --- UI ---
  QWidget central;
  auto* layout = new QVBoxLayout(&central);

  QCheckBox rotateCheck(QStringLiteral("自动旋转"));
  rotateCheck.setChecked(true);

  QVTKOpenGLNativeWidget widget;
  widget.setRenderWindow(renderWindow.Get());

  layout->addWidget(&rotateCheck);
  layout->addWidget(&widget);

  // auto-rotate: rotate the camera a bit each frame while the box is checked
  QTimer rotateTimer;
  QObject::connect(&rotateTimer, &QTimer::timeout, [&]() {
    if (rotateCheck.isChecked())
    {
      renderer->GetActiveCamera()->Azimuth(0.5);
      renderer->ResetCameraClippingRange();
      renderWindow->Render();
    }
  });
  rotateTimer.start(16); // ~60 fps

  central.resize(800, 600);
  central.show();

  return app.exec();
}
