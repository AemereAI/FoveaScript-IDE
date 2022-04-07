#include "fovea3d.h"

Fovea3D::~Fovea3D()
{

}

Fovea3D::Fovea3D(QWidget *parent, int w, int h) : QOpenGLWidget(parent)
{

    overrideScale = 0.0;
    vertex_shader = const_cast<GLchar*>( "attribute highp vec3 position;\n"
                    "attribute highp vec3 normal;\n"
                    "attribute highp vec3 color;\n"
                    "varying vec4 frag_color;\n"
                    "uniform highp mat4 matrix;\n"
                    "uniform vec3 light_pos_1;\n"
                    "uniform vec3 light_pos_2;\n"
                    "uniform vec3 light_pos_3;\n"
                    "uniform vec3 diffuse_light;\n"
                    "uniform vec3 specular_light;\n"
                    "vec3 light_calc(vec3 lightpos,vec3 pos,vec3 norm){\n"
                    "vec3 light_direction = normalize(lightpos - pos);\n"
                    "float diffuse_coeff = 200.0 * clamp(dot(light_direction,norm),0.1,1.0);\n"
                    "vec3 reflect_direction = reflect(-light_direction,norm);\n"
                    "float specular_coeff = 1000.0* pow(max(0.0,dot(light_direction,reflect_direction)),0.2);\n"
                    "vec3 lighting = normalize(specular_coeff*specular_light + diffuse_coeff*diffuse_light);\n"
                    "return lighting;}\n"
                    "void main(){\n"
                    "gl_Position = matrix * vec4(position,1.0);\n"
                    "vec3 lighting = light_calc(light_pos_1,position,normal);\n"
                    "lighting += light_calc(light_pos_2,position,normal);\n"
                    "lighting += 0.7*light_calc(light_pos_3,position,normal);\n"
                    "frag_color = vec4(color,1.0)*vec4(lighting,1.0);\n}\0");
    fragment_shader = const_cast<GLchar*>( "varying vec4 frag_color;\n"
                    "void main(){\n"
                    "gl_FragColor= frag_color;}\0");

    width= w;
    height= h;
    paintDevice = nullptr;
    initialized = false;
    currentScaledValueIndex = 0;

    read = new OBJReader;
    if(!read->setModelFile(":/eye.obj",":/eye.mtl"))
        return;
    try{
        read->processFile();
    }
    catch(int err)
    {
        if(err == NOMATERIAL || err == -1)
            return;
    }

}

void Fovea3D::setSize(int x,int y){
width = x;
height = y;
}
void Fovea3D::initDataObj()
{

    QList<Primitive*>* polygon = read->getAllPolyVertices();
    int size = static_cast<int>(polygon->length()*(polygon->at(0)->setting)*3*1.5); //1.5 for duplications (square to two triangles)
    dataVertex = new GLfloat[size];
    dataColor = new GLfloat[size];
    dataNormal = new GLfloat[size];
    count = size/3; //3 values per vertex
    eyeBallPolygonCount = 0;
    for(int ind=0; ind<polygon->length();ind++)
   {
        //Color 3 floats,then 3 Vertex floats
        //Modify square for GL_TRIANGLES

        dataColor[ind*18] = static_cast<GLfloat>(read->getMaterials(polygon->at(ind)->materialIndex)->red);
        dataColor[ind*18+1] = static_cast<GLfloat>( read->getMaterials(polygon->at(ind)->materialIndex)->green);
        dataColor[ind*18+2] = static_cast<GLfloat>(read->getMaterials(polygon->at(ind)->materialIndex)->blue);
        dataVertex[ind*18] = static_cast<GLfloat>(polygon->at(ind)->x[0]);
        dataVertex[ind*18+1] =static_cast<GLfloat>(polygon->at(ind)->y[0]);
        dataVertex[ind*18+2] =static_cast<GLfloat>(polygon->at(ind)->z[0]);
        dataNormal[ind*18] = static_cast<GLfloat>(polygon->at(ind)->x[3]);
        dataNormal[ind*18+1] =static_cast<GLfloat>(polygon->at(ind)->y[3]);
        dataNormal[ind*18+2] =static_cast<GLfloat>(polygon->at(ind)->z[3]);

        dataColor[ind*18+3] = static_cast<GLfloat>(read->getMaterials(polygon->at(ind)->materialIndex)->red);
        dataColor[ind*18+4] = static_cast<GLfloat>(read->getMaterials(polygon->at(ind)->materialIndex)->green);
        dataColor[ind*18+5] = static_cast<GLfloat>(read->getMaterials(polygon->at(ind)->materialIndex)->blue);
        dataVertex[ind*18+3] =static_cast<GLfloat>(polygon->at(ind)->x[1]);
        dataVertex[ind*18+4] =static_cast<GLfloat>(polygon->at(ind)->y[1]);
        dataVertex[ind*18+5] =static_cast<GLfloat>(polygon->at(ind)->z[1]);
        dataNormal[ind*18+3] = static_cast<GLfloat>(polygon->at(ind)->x[4]);
        dataNormal[ind*18+4] =static_cast<GLfloat>(polygon->at(ind)->y[4]);
        dataNormal[ind*18+5] =static_cast<GLfloat>(polygon->at(ind)->z[4]);

        dataColor[ind*18+6] = static_cast<GLfloat>(read->getMaterials(polygon->at(ind)->materialIndex)->red);
        dataColor[ind*18+7] = static_cast<GLfloat>(read->getMaterials(polygon->at(ind)->materialIndex)->green);
        dataColor[ind*18+8] = static_cast<GLfloat>(read->getMaterials(polygon->at(ind)->materialIndex)->blue);
        dataVertex[ind*18+6] =static_cast<GLfloat>(polygon->at(ind)->x[3]);
        dataVertex[ind*18+7] = static_cast<GLfloat>(polygon->at(ind)->y[3]);
        dataVertex[ind*18+8] = static_cast<GLfloat>(polygon->at(ind)->z[3]);
        dataNormal[ind*18+6] = static_cast<GLfloat>(polygon->at(ind)->x[6]);
        dataNormal[ind*18+7] =static_cast<GLfloat>(polygon->at(ind)->y[6]);
        dataNormal[ind*18+8] =static_cast<GLfloat>(polygon->at(ind)->z[6]);

        dataColor[ind*18+9] = static_cast<GLfloat>(read->getMaterials(polygon->at(ind)->materialIndex)->red);
        dataColor[ind*18+10] = static_cast<GLfloat>(read->getMaterials(polygon->at(ind)->materialIndex)->green);
        dataColor[ind*18+11] = static_cast<GLfloat>(read->getMaterials(polygon->at(ind)->materialIndex)->blue);
        dataVertex[ind*18+9] =static_cast<GLfloat>(polygon->at(ind)->x[1]);
        dataVertex[ind*18+10] =static_cast<GLfloat>(polygon->at(ind)->y[1]);
        dataVertex[ind*18+11] =static_cast<GLfloat>(polygon->at(ind)->z[1]);
        dataNormal[ind*18+9] = static_cast<GLfloat>(polygon->at(ind)->x[4]);
        dataNormal[ind*18+10] =static_cast<GLfloat>(polygon->at(ind)->y[4]);
        dataNormal[ind*18+11] =static_cast<GLfloat>(polygon->at(ind)->z[4]);

        dataColor[ind*18+12] = static_cast<GLfloat>(read->getMaterials(polygon->at(ind)->materialIndex)->red);
        dataColor[ind*18+13] = static_cast<GLfloat>(read->getMaterials(polygon->at(ind)->materialIndex)->green);
        dataColor[ind*18+14] = static_cast<GLfloat>(read->getMaterials(polygon->at(ind)->materialIndex)->blue);
        dataVertex[ind*18+12] =static_cast<GLfloat>( polygon->at(ind)->x[2]);
        dataVertex[ind*18+13] =static_cast<GLfloat>(polygon->at(ind)->y[2]);
        dataVertex[ind*18+14] =static_cast<GLfloat>(polygon->at(ind)->z[2]);
        dataNormal[ind*18+12] = static_cast<GLfloat>(polygon->at(ind)->x[5]);
        dataNormal[ind*18+13] =static_cast<GLfloat>(polygon->at(ind)->y[5]);
        dataNormal[ind*18+14] =static_cast<GLfloat>( polygon->at(ind)->z[5]);

        dataColor[ind*18+15] = static_cast<GLfloat>(read->getMaterials(polygon->at(ind)->materialIndex)->red);
        dataColor[ind*18+16] = static_cast<GLfloat>(read->getMaterials(polygon->at(ind)->materialIndex)->green);
        dataColor[ind*18+17] = static_cast<GLfloat>(read->getMaterials(polygon->at(ind)->materialIndex)->blue);
        dataVertex[ind*18+15] =static_cast<GLfloat>(polygon->at(ind)->x[3]);
        dataVertex[ind*18+16] =static_cast<GLfloat>(polygon->at(ind)->y[3]);
        dataVertex[ind*18+17] =static_cast<GLfloat>(polygon->at(ind)->z[3]);
        dataNormal[ind*18+15] = static_cast<GLfloat>(polygon->at(ind)->x[6]);
        dataNormal[ind*18+16] =static_cast<GLfloat>(polygon->at(ind)->y[6]);
        dataNormal[ind*18+17] =static_cast<GLfloat>( polygon->at(ind)->z[6]);


        if(read->getMaterials(polygon->at(ind)->materialIndex)->name == "EYEBALL")
        {
            eyeBallPolygonCount++;
        }
    }

}

void Fovea3D::animate(int type)
{
    animateSet = type;
}

void Fovea3D::initializeGL()
{
    initializeOpenGLFunctions();
    create();
    makeCurrent();
    //Set Up Scene
    initShaders();
    initDataObj();
}


void Fovea3D::setViewMatrix()
{
    modelView = new QMatrix4x4;
    //modelView->perspective(90.0f, 4.0f/3.0f, 0, 100.0f);
    modelView->setToIdentity();
    float scaleVal = static_cast<float>(initialScaledValue * pow(1.5,currentScaledValueIndex));
    if(overrideScale > 0.0)
    {
        scaleVal = static_cast<float>(overrideScale);
    }
    modelView->scale(scaleVal,scaleVal,scaleVal);
    modelView->translate(2.0,-1.0,0.0);
    modelView->rotate(QQuaternion::fromAxisAndAngle(0,1,0,55));
    prog.setUniformValue(viewUniformLoc,*modelView);
    setLighting();
}

void Fovea3D::setScale(float scale)
{
    overrideScale = static_cast<double>(scale);
}
void Fovea3D::setLighting()
{
    prog.setUniformValue(posUniformLightLoc,static_cast<GLfloat>(0.0),static_cast<GLfloat>(0.5),static_cast<GLfloat>(0.3));
    prog.setUniformValue(posUniformLightLoc2,static_cast<GLfloat>(0.7),static_cast<GLfloat>(0.25),static_cast<GLfloat>(0.6));
    prog.setUniformValue(posUniformLightLoc3,static_cast<GLfloat>(-0.6),static_cast<GLfloat>(0.3),static_cast<GLfloat>(0.5));
    prog.setUniformValue(lightUniformDiffuseLoc,1.0,0.75,0.75);
    prog.setUniformValue(lightUniformSpecularLoc,1.0,0.75,0.75); //0.551
}

GLuint Fovea3D::initShaders()
{
    if(!prog.addShaderFromSourceCode(QOpenGLShader::Vertex,vertex_shader))
        close();
    if(!prog.addShaderFromSourceCode(QOpenGLShader::Fragment,fragment_shader))
        close();
    prog.link();
    posAttrLoc = prog.attributeLocation("position");
    colAttrLoc = prog.attributeLocation("color");
    normAttrLoc = prog.attributeLocation("normal");
    viewUniformLoc = prog.uniformLocation("matrix");
    posUniformLightLoc = prog.uniformLocation("light_pos_1");
    posUniformLightLoc2 = prog.uniformLocation("light_pos_2");
    posUniformLightLoc3 = prog.uniformLocation("light_pos_3");
    lightUniformDiffuseLoc = prog.uniformLocation("diffuse_light");
    lightUniformSpecularLoc = prog.uniformLocation("specular_light");
    return 0;
}

void Fovea3D::renderUpdater()
{
    static int repCount = 0;
    static int RGB = 0; //Must animate in mutiples of three
    if(animateSet == LOAD_BRING_IN)
    {
        if(repCount++ >= 2*17){//2^10 factors increase to 0.15 taking into account zero indexing and subsequent 1 magnification
            animateSet = 0;
            repCount = 0;
        }
        if(repCount % 2)
            currentScaledValueIndex++;
    }
    if(animateSet == LOAD_GO)
    {
        if(repCount-- == 1){//2^10 factors increase to 0.15 taking into account zero indexing and subsequent 1 magnification
            animateSet = 0;
            repCount = 0;
        }
        if(repCount % 2)
            currentScaledValueIndex--;

    }
    if(animateSet == PROGRESS_ROTATE ||(RGB % 3))
    {  

        read->swizzlePupil();
        RGB++;
        if(RGB > 10000)
            RGB = RGB % 3;
        for(auto i{0}; i < 100000; i++)
        {
            QCoreApplication::processEvents();
        }
        initDataObj();
    }
}

void Fovea3D::paintGL()
{
    render();
}

void Fovea3D::paintEvent(QPaintEvent *e)
{
        renderUpdater();
        QOpenGLWidget::paintEvent(e);
}

void Fovea3D::render(QPainter *painter)
{
    Q_UNUSED(painter);

}

void Fovea3D::render()
{
    if(!paintDevice)
        paintDevice = new QOpenGLPaintDevice;
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    paintDevice->setSize(size());
    QPainter painter(paintDevice);
    render(&painter);

    glViewport(0,0,width,height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glClearColor(0.976,0.976,0.976,1.0);
    glFrontFace(GL_CW);
    glEnable(GL_CULL_FACE);
    prog.bind();
    setViewMatrix();
    glVertexAttribPointer(static_cast<GLuint>(posAttrLoc),3,GL_FLOAT,GL_TRUE,0,static_cast<void*>(dataVertex));
    glVertexAttribPointer(static_cast<GLuint>(colAttrLoc),3,GL_FLOAT,GL_TRUE,0,static_cast<void*>(dataColor));
    glVertexAttribPointer(static_cast<GLuint>(normAttrLoc),3,GL_FLOAT,GL_FALSE,0,static_cast<void*>(dataNormal));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    int verticesCountEyeBall = eyeBallPolygonCount*6;
    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
    glDrawArrays(GL_TRIANGLES,0,verticesCountEyeBall);
    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
    glDrawArrays(GL_TRIANGLES,verticesCountEyeBall,count-verticesCountEyeBall);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(0);
    prog.release();
    update();
}


