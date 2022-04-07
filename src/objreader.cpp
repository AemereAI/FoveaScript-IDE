#include "objreader.h"

OBJReader::OBJReader()
{
    vertexInfo = new QList<Primitive*>();
    materials = new QList<Material*>();
}

bool OBJReader::setModelFile(QString OBJfile, QString MTLs)
{
    obj_file = new QFile(OBJfile);
    mtl_file = new QFile(MTLs);
    if(!obj_file->open(QIODevice::ReadOnly) || !mtl_file->open(QIODevice::ReadOnly))
    {
        QString error = obj_file->errorString();
        return false;
    }
    else{
        QTextStream text(obj_file);
        obj_content = text.readAll();
        obj_file->close();
        QTextStream text2(mtl_file);
        mtl_content = text2.readAll();
        mtl_file->close();
        return true;
    }
}

bool OBJReader::processFile()
{
 QRegularExpression regex(pat_vertices);
 QString vertices = regex.match(obj_content).captured(1);
 QRegularExpression regexNormals(pat_vertex_normal);
 QString verticesNormal = regexNormals.match(obj_content).captured(1);
 QStringList vArray = vertices.replace("v","").split("\n");
 QStringList vnArray = verticesNormal.replace("vn","").split("\n");

 QRegularExpression regexFaces(pat_faces);
 QRegularExpressionMatch match = regexFaces.match(obj_content,QRegularExpression::MultilineOption);
 QString facesMtl= match.captured(1);
 QString facesV = match.captured(2);

 QRegularExpression regexMtl(pat_materials);
 //Obtain materials
QRegularExpressionMatchIterator materialNameIterator = regexMtl.globalMatch(mtl_content);

//Determine size of mtl
//For now skip
while(materialNameIterator.hasNext())
{
    QRegularExpressionMatch materialName = materialNameIterator.next();
    Material* mtltemp = new Material;
    QStringList lines = materialName.captured(2).split("\n");
    QString colorLineString = lines[2];
    QStringList colorLine = colorLineString.split(" ");
    mtltemp->name = materialName.captured(1);
    mtltemp->red = colorLine.at(1).toFloat();
    mtltemp->green = colorLine.at(2).toFloat();
    mtltemp->blue = colorLine.at(3).toFloat();
    materials->append(mtltemp);
}

//Store the material  used for this and the consecutive data point therein
//Extract and store model data for access
Primitive* polygonData = new Primitive(GENERICPOLY,-1);
QRegularExpressionMatchIterator matchIter = regexFaces.globalMatch((obj_content));

 while(matchIter.hasNext())
 {
    QRegularExpressionMatch match = matchIter.next();
    facesMtl= match.captured(1);
    facesV = match.captured(2);
    QStringList f {facesV.split("\n")};

    for(int mtlindex=0; mtlindex < materials->length(); mtlindex++)
    {
        if(materials->at(mtlindex)->name.compare(facesMtl) == 0)
            polygonData->materialIndex = mtlindex;
    }
    for(int i = 0; i < f.length();i++)
    {
       QString line { f.at(i) };
       QStringList polygon = line.split(" ");

       if(polygonData->materialIndex != -1)
       {
           int indexMtl = polygonData->materialIndex;

           if(polygon.length()-1 > PENTAGON)
               throw -1;
           polygonData = new Primitive((enum PrimitiveType)(polygon.length()-1),indexMtl);
       }
       else
       {
           throw NOMATERIAL;
       }


       for(int j = 1; j < (polygon.length());j++)
       {
           QString value {polygon.at(j)};
           QString temp_1= value.split("//").at(0);
           QString temp_2= value.split("//").at(1);
           int value_1 = temp_1.toInt();
           int value_2 = temp_2.toInt();

           QString vertexData = vArray.at(value_1-1);
           QString vertexNormalData = vnArray.at(value_2-1);
           polygonData->x[j-1] = vertexData.split(" ").at(1).toFloat();
           polygonData->x[(j-1)+polygonData->setting] = vertexNormalData.split(" ").at(1).toFloat();
           polygonData->y[j-1] = vertexData.split(" ").at(2).toFloat();
           polygonData->y[(j-1)+polygonData->setting] = vertexNormalData.split(" ").at(2).toFloat();
           polygonData->z[j-1] = vertexData.split(" ").at(3).toFloat();
           polygonData->z[(j-1)+polygonData->setting] =vertexNormalData.split(" ").at(3).toFloat();
       }
       vertexInfo->push_back(polygonData);
    }
    if(polygonData->materialIndex != -1)
    {
        int indexMtl = polygonData->materialIndex;

        polygonData = new Primitive((enum PrimitiveType)(polygonData->setting),indexMtl);
    }
    else
    {
        throw NOMATERIAL;
    }
 }
 return true;
}

Primitive* OBJReader::getNextPolyVertices(bool Reset) const
{
    static int index;

    if(Reset)
        index = 0;
    if(index >= vertexInfo->length())
        return nullptr;
    return vertexInfo->at(index++);
}

QList<Primitive* >* OBJReader::getAllPolyVertices() const
{
    return vertexInfo;
}

Material* OBJReader::getMaterials(int Index) const
{
    return materials->at(Index);
}

bool OBJReader::swizzlePupil()
{
    float redPix[3];
    float bluePix[3];
    float greenPix[3];

    for(int i = 0; i< materials->length(); i++)
    {
        if(materials->at(i)->name == "RedPix")
        {
                redPix[0] = materials->at(i)->red;
                redPix[1] = materials->at(i)->green;
                redPix[2] = materials->at(i)->blue;
        }
        if(materials->at(i)->name == "BluePix")
        {
                bluePix[0] = materials->at(i)->red;
                bluePix[1] = materials->at(i)->green;
                bluePix[2] = materials->at(i)->blue;
         }
         if(materials->at(i)->name == "GreenPix")
         {
                greenPix[0] = materials->at(i)->red;
                greenPix[1] = materials->at(i)->green;
                greenPix[2] = materials->at(i)->blue;
        }
    }
    for(int i = 0; i < materials->length(); i++)
    {

            if(materials->at(i)->name == "RedPix")
            {
                materials->at(i)->red = greenPix[0];
                materials->at(i)->green = greenPix[1];
                materials->at(i)->blue = greenPix[2];

             }
            if(materials->at(i)->name == "BluePix")
            {
                materials->at(i)->red = redPix[0];
                materials->at(i)->green = redPix[1];
                materials->at(i)->blue = redPix[2];
             }
            if(materials->at(i)->name == "GreenPix")
            {
                materials->at(i)->red = bluePix[0];
                materials->at(i)->green = bluePix[1];
                materials->at(i)->blue = bluePix[2];
             }
        }
    return true;
}
