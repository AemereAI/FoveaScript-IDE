#ifndef OBJREADER_H
#define OBJREADER_H
//Note: this class is not suited to generic use only temporary

#include <QFile>
#include <QTextStream>
#include <QString>
#include <QStringList>
#include <GLES3/gl3.h>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QList>

#define NOMATERIAL 88
enum PrimitiveType {TRIANGLE=3,GENERICPOLY=4,PENTAGON=5};

class Primitive
{
  public:
    Primitive(enum PrimitiveType type = TRIANGLE,int material = 0)
    {
        setting = type;
        //Store vector and vector normal points in one array
        x = new GLfloat[type*2];
        y = new GLfloat[type*2];
        z = new GLfloat[type*2];
        materialIndex = material;
    }
    enum PrimitiveType setting;
    GLfloat* x; //X coordinate for vertices according to type
    GLfloat* y;
    GLfloat* z;
    int materialIndex;
};

class Material
{
public:
   QString name;
   GLfloat red;
   GLfloat green;
   GLfloat blue;
};

class OBJReader
{
public:
    OBJReader();
    bool setModelFile(QString OBJFile, QString MTLs);
    bool processFile();
    //These get get methods provide vertex data arranged together
    Primitive* getNextPolyVertices(bool Reset) const;
    QList<Primitive*>* getAllPolyVertices() const;
    Material* getMaterials(int Index) const;
    bool swizzlePupil();
private:
    GLfloat* v;
    GLfloat* vn;

    const char* pat_vertices = "\n(v[^A-Za-uw-z]+)";
    const char* pat_vertex_normal = "\n(vn[^fu)]+)";
    const char* pat_faces = "usemtl ([^\n]+)(?:\ns [0-9]+)?\n(f[f 0-9/\n]+)\n";
    const char* pat_materials = "newmtl ([A-Za-z0-9]+)\n([^l]+)";
    QFile* obj_file;
    QFile* mtl_file;
    QString obj_content;
    QString mtl_content;
    QList<Material*>* materials;
    QList<Primitive*>* vertexInfo;
};

#endif // OBJREADER_H
