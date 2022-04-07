#ifndef FOVEA3D_H
#define FOVEA3D_H

#include <QOpenGLWidget>
#include <QtGui/QGuiApplication>
#include <QtGui/QMatrix4x4>
#include <QtGui/QOpenGLShaderProgram>
#include <QtGui/QOpenGLPaintDevice>
#include <QtGui/QScreen>
#include <QtGui/QPainter>
#include <QtGui/QWindow>
#include <QtGui/QOpenGLFunctions>
#include <QtCore/qmath.h>
#include <QTextStream>
#include <QtMath>
#include "objreader.h"

#define PROGRESS 100
class Fovea3D : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT
public:
    static const int PROGRESS_ROTATE = 100;
    static const int LOAD_BRING_IN = 111;
    static const int LOAD_GO = 122;
    explicit Fovea3D(QWidget *parent = nullptr,int w = 500, int h = 500);
    virtual ~Fovea3D();
    void initDataObj();
    void setViewMatrix();
    void setSize(int x,int y);
    void setScale(float scale);
    GLuint initShaders();
    void initializeGL();
    void render();
    void paintGL();
    void render(QPainter *painter);
    void animate(int type);
    void setLighting();
signals:

public slots:
    void renderUpdater();
protected:
    void paintEvent(QPaintEvent *e);

private:
    OBJReader* read;
    QOpenGLContext* context;
    QOpenGLPaintDevice* paintDevice;
    QMatrix4x4* modelView;
    const char* vertex_shader;
    const char* fragment_shader;
    QOpenGLShaderProgram prog;
    int posAttrLoc;
    int colAttrLoc;
    int normAttrLoc;
    int viewUniformLoc;
    int lightUniformDiffuseLoc;
    int lightUniformSpecularLoc;
    int posUniformLightLoc;
    int posUniformLightLoc2;
    int posUniformLightLoc3;
    GLfloat* dataVertex;
    GLfloat* dataColor;
    GLfloat* dataNormal;
    int count;
    int eyeBallPolygonCount;
    bool initialized;
    int animateSet;
    const double initialScaledValue = 0.0001465;
    double overrideScale;
    int currentScaledValueIndex;
    int width;
    int height;
};

#endif // FOVEA3D_H
