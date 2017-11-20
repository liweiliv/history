/*
 * ball.cpp
 *
 *  Created on: 2017年11月13日
 *      Author: liwei
 */




////////////////////////////////////////////////////////////
// GLsphere.h
//
////////////////////////////////////////////////////////////
#pragma once
#include <vector>
#include <math.h>
using std::vector;


// 浮点向量
struct VECTOR_F{
    VECTOR_F(const float &xparam, const float &yparam, const float &zparam)
    :x(xparam), y(yparam), z(zparam)
    {}

    float x, y, z;
};
// 3维点
struct POINT3D_F{
    float x, y, z;
    POINT3D_F *a, *b, *c, *d, *e, *f;
};
// 3维三角形
struct TRIANGLE_F{
    // 三个顶点
    POINT3D_F *a, *b, *c;
    // 三角形相对于圆心法线,单位向量
    float normal[3];
};


class CSphere{
public:
    CSphere(const int &detialLevel, const float &sphereSize);
    ~CSphere();
    void release();
    bool isSuccess() { return issuccess; }
private:
    int private_power(const int &m, const int &n);          // 求冪
    // 计算指定复杂度存储所有顶点需要的行数
    int private_calNumofRows(const int &detailLevel);
    // 计算指定复杂度下某行顶点数量
    int private_calPointsoftheRow(const int &detailLevel, const int &rowNumber);
    // 计算指定复杂度下某行三角形数量
    int private_calTriangleoftheRow(const int &dl, const int &rowNumber);
    int private_calRowX(const int &detailLevel);
    // 取得vector<POINT3D_F*>指定索引元素的迭代器（非安全）
    vector<POINT3D_F*>::iterator private_getVectorofPoints_iterator(vector<POINT3D_F*> &pointsArray,int &index);
    // 取得vector<vector<POINT3D_F*>>指定索引数组的迭代器（非安全）
    vector<vector<POINT3D_F*>>::iterator private_getVectorofVectorofPoints_iterator(
        vector<vector<POINT3D_F*>> &vectorArray, int &index
        );
    // 取得指定点的编号A邻居
    POINT3D_F* private_getNeighborA(int curDL, int rowindex, int elementindex);
    // 取得指定点的编号B邻居
    POINT3D_F* private_getNeighborB(int curDL, int rowindex, int elementindex);
    // 取得指定点的编号C邻居
    POINT3D_F* private_getNeighborC(int curDL, int rowindex, int elementindex);
    // 取得指定点的编号D邻居
    POINT3D_F* private_getNeighborD(int curDL, int rowindex, int elementindex);
    // 取得指定点的编号E邻居
    POINT3D_F* private_getNeighborE(int curDL, int rowindex, int elementindex);
    // 取得指定点的编号F邻居
    POINT3D_F* private_getNeighborF(int curDL, int rowindex, int elementindex);
    // 计算两向量的中间向量
    VECTOR_F private_calMidVector(const VECTOR_F &v1, const VECTOR_F &v2);
    // 根据三角形三个顶点计算三角形法线
    void private_initTriangleNormal(TRIANGLE_F *triangle);
public:
    bool issuccess;         // 是否初始化成功
    int dl;
    float size;

    int pointsCount;        // 顶点数量：6+(n-1)*3
    vector<vector<POINT3D_F *>> points;
    int trianglesCount;     // 三角形数量：4^detailLevel
    vector<vector<TRIANGLE_F*>> triangles;
};

/*
* 构造
* @detailLevel:细分等级，大于等于2
* @size:球半径
*/
CSphere::CSphere(const int &detailLevel, const float &sphereSize)
:dl(0), size(0.0f), pointsCount(0), trianglesCount(0)
{
    if ( (detailLevel < 2)||(sphereSize <= 0.0f) )
    {
        issuccess = false;
    }else{
        dl = detailLevel;
        size = sphereSize;

        // 手动插入当复杂度为1时的点
        vector<POINT3D_F *> temp;
        points.push_back(temp);
        points.push_back(temp);
        points.push_back(temp);
        POINT3D_F *newpoint = nullptr;
        // 上
        newpoint = (POINT3D_F*)malloc(sizeof(POINT3D_F));
        newpoint->x = newpoint->z = 0.0f;
        newpoint->y = size;
        points[0].push_back(newpoint);
        // 前
        newpoint = (POINT3D_F*)malloc(sizeof(POINT3D_F));
        newpoint->x = newpoint->y = 0.0f;
        newpoint->z = size;
        points[1].push_back(newpoint);
        // 右
        newpoint = (POINT3D_F*)malloc(sizeof(POINT3D_F));
        newpoint->y = newpoint->z = 0.0f;
        newpoint->x = size;
        points[1].push_back(newpoint);
        // 后
        newpoint = (POINT3D_F*)malloc(sizeof(POINT3D_F));
        newpoint->x = newpoint->y = 0.0f;
        newpoint->z = -size;
        points[1].push_back(newpoint);
        // 左
        newpoint = (POINT3D_F *)malloc(sizeof(POINT3D_F));
        newpoint->y = newpoint->z = 0.0f;
        newpoint->x = -size;
        points[1].push_back(newpoint);
        // 下
        newpoint = (POINT3D_F*)malloc(sizeof(POINT3D_F));
        newpoint->x = newpoint->z = 0.0f;
        newpoint->y = -size;
        points[2].push_back(newpoint);
        // 开始插值,递增复杂度
        for (int curDL = 2; curDL <= dl;++curDL)
        {
            // 第一步：对已存在的行进行插值
            int rowMax = private_calNumofRows(curDL-1) - 2;
            // 开始逐行插值
            for (int rowindex = 1; rowindex <= rowMax;++rowindex)
            {
                // 逐行间隔插入新点
                for (int elementindex = 0; (unsigned)elementindex < points[rowindex].size(); ++++elementindex)
                {
                    // 初始化一个新点
                    newpoint = (POINT3D_F *)malloc(sizeof(POINT3D_F));
                    int lindex = elementindex;
                    int rindex = (elementindex + 1) % points[rowindex].size();
                    VECTOR_F newv = private_calMidVector(
                        VECTOR_F( points[rowindex][lindex]->x, points[rowindex][lindex]->y, points[rowindex][lindex]->z ),
                        VECTOR_F( points[rowindex][rindex]->x, points[rowindex][rindex]->y, points[rowindex][rindex]->z )
                        );
                    // 根据圆心到点有向线段的单位向量计算点的坐标
                    newpoint->x = newv.x * size;
                    newpoint->y = newv.y * size;
                    newpoint->z = newv.z * size;
                    // 插入点
                    points[rowindex].insert(private_getVectorofPoints_iterator(points[rowindex], elementindex) + 1, newpoint);
                }
            }
            // 第二步：插入整行的点
            rowMax = private_calNumofRows(curDL) - 1;
            for (int rowindex = 0; rowindex < rowMax;)
            {
                // 在当前行索引之后插入一个空行，并递增索引指向新插入的行
                ++rowindex;     // rowindex指向新插入的行
                points.insert(private_getVectorofVectorofPoints_iterator(points, rowindex), temp);
                // 初始化当前行
                int elementMax = private_calPointsoftheRow(curDL, rowindex);
                int preElementMax = private_calPointsoftheRow(curDL, rowindex-1 );
                int nextElementMax = private_calPointsoftheRow(curDL, rowindex+1 );
                for (int elementindex = 0; elementindex < elementMax; ++elementindex)
                {
                    POINT3D_F *prePoint = nullptr;
                    POINT3D_F *nextPoint = nullptr;
                    if (0 == elementindex)
                    {
                        prePoint = points[rowindex - 1][0];
                        nextPoint = points[rowindex + 1][0];
                    }else{
                        if (rowindex < private_calNumofRows(curDL)/2)
                        {
                            prePoint = private_getNeighborA(curDL, rowindex, elementindex);
                            nextPoint = private_getNeighborD(curDL, rowindex, elementindex);
                        }else{
                            prePoint = private_getNeighborF(curDL, rowindex, elementindex);
                            nextPoint = private_getNeighborC(curDL, rowindex, elementindex);
                        }

                    }
                    // 计算圆心到新插值点有向线段的单位向量
                    VECTOR_F newv = private_calMidVector( VECTOR_F(prePoint->x, prePoint->y, prePoint->z), VECTOR_F(nextPoint->x, nextPoint->y, nextPoint->z) );
                    // 新点
                    newpoint = (POINT3D_F *)malloc(sizeof(POINT3D_F));
                    newpoint->x = newv.x * size;
                    newpoint->y = newv.y * size;
                    newpoint->z = newv.z * size;

                    points[rowindex].push_back(newpoint);
                }
                // 再次递增索引
                ++rowindex;
            }
        }

        // 手动初始化北极点指针成员
        points[0][0]->a = private_getNeighborA(dl, 0, 0);
        points[0][0]->b = private_getNeighborB(dl, 0, 0);
        points[0][0]->c = private_getNeighborC(dl, 0, 0);
        points[0][0]->d = private_getNeighborD(dl, 0, 0);
        points[0][0]->e = points[0][0]->f = nullptr;
        // 手动初始化南极点成员
        int maxRowIndex = private_calNumofRows(dl) - 1;
        points[maxRowIndex][0]->a = private_getNeighborA(dl, maxRowIndex, 0);
        points[maxRowIndex][0]->b = private_getNeighborB(dl, maxRowIndex, 0);
        points[maxRowIndex][0]->c = private_getNeighborC(dl, maxRowIndex, 0);
        points[maxRowIndex][0]->d = private_getNeighborD(dl, maxRowIndex, 0);
        points[maxRowIndex][0]->e = points[maxRowIndex][0]->f = nullptr;
        // 初始化顶点指针成员
        for (int rowindex = 0; (unsigned)rowindex < points.size();++rowindex)
        {
            for (int elementindex = 0; (unsigned)elementindex < points[rowindex].size(); ++elementindex)
            {
                points[rowindex][elementindex]->a = private_getNeighborA(dl, rowindex, elementindex);
                points[rowindex][elementindex]->b = private_getNeighborB(dl, rowindex, elementindex);
                points[rowindex][elementindex]->c = private_getNeighborC(dl, rowindex, elementindex);
                points[rowindex][elementindex]->d = private_getNeighborD(dl, rowindex, elementindex);
                points[rowindex][elementindex]->e = private_getNeighborE(dl, rowindex, elementindex);
                points[rowindex][elementindex]->f = private_getNeighborF(dl, rowindex, elementindex);
            }
        }

        vector<TRIANGLE_F*> trians;
        TRIANGLE_F *newtrian = nullptr;
        // 手动插入第一行三角形数据(全部为正三角形)
        triangles.push_back(trians);
        newtrian = (TRIANGLE_F*)malloc(sizeof(TRIANGLE_F));
        newtrian->a = points[0][0];
        newtrian->b = points[1][0];
        newtrian->c = points[1][1];
        private_initTriangleNormal(newtrian);
        triangles[0].push_back(newtrian);
        newtrian = (TRIANGLE_F*)malloc(sizeof(TRIANGLE_F));
        newtrian->a = points[0][0];
        newtrian->b = points[1][1];
        newtrian->c = points[1][2];
        private_initTriangleNormal(newtrian);
        triangles[0].push_back(newtrian);
        newtrian = (TRIANGLE_F*)malloc(sizeof(TRIANGLE_F));
        newtrian->a = points[0][0];
        newtrian->b = points[1][2];
        newtrian->c = points[1][3];
        private_initTriangleNormal(newtrian);
        triangles[0].push_back(newtrian);
        newtrian = (TRIANGLE_F*)malloc(sizeof(TRIANGLE_F));
        newtrian->a = points[0][0];
        newtrian->b = points[1][3];
        newtrian->c = points[1][0];
        private_initTriangleNormal(newtrian);
        triangles[0].push_back(newtrian);
        // 生成第一行和最后一行外其他的行的三角形数据
        maxRowIndex = private_calNumofRows(dl) - 3;
        for (int rowindex = 1; rowindex <= maxRowIndex;++rowindex)
        {
            triangles.push_back(trians);
            int maxElementIndex = private_calTriangleoftheRow(dl, rowindex) - 1;
            int midflag = (private_calNumofRows(dl) - 1) / 2;
            for (int elementindex = 0; elementindex <= maxElementIndex;++elementindex)
            {
                newtrian = (TRIANGLE_F*)malloc(sizeof(TRIANGLE_F));
                if (rowindex < midflag)// 如果位于上半球
                {
                    if (0 == elementindex)// 如果索引为0(上半球索引为0的三角形都是正三角)
                    {
                        newtrian->a = points[rowindex][0];
                        newtrian->b = points[rowindex + 1][0];
                        newtrian->c = points[rowindex + 1][1];
                    }else{
                        int groupid = elementindex / (rowindex * 2 + 1);
                        if ((groupid % 2 == 0 && elementindex % 2 == 0)||(groupid % 2 != 0 && elementindex % 2 != 0))
                        {// 如果是正三角
                            int ax = (elementindex - groupid) / 2 % private_calPointsoftheRow(dl, rowindex);
                            int ay = rowindex;
                            newtrian->a = points[ay][ax];
                            int bx = (elementindex - groupid) / 2 + groupid;
                            int by = ay + 1;
                            newtrian->b = points[by][bx];
                            int cx = (bx + 1) % private_calPointsoftheRow(dl, rowindex+1);
                            int cy = by;
                            newtrian->c = points[cy][cx];
                        }else{// 如果是倒三角
                            int cx = (elementindex + 1 - groupid)/2 % private_calPointsoftheRow(dl, rowindex);
                            int cy = rowindex;
                            newtrian->c = points[cy][cx];
                            int ax = ((elementindex + 1 - groupid) / 2 - 1 + private_calPointsoftheRow(dl, rowindex)) % private_calPointsoftheRow(dl, rowindex);
                            int ay = cy;
                            newtrian->a = points[ay][ax];
                            int bx = (elementindex + 1 - groupid) / 2 + groupid;
                            int by = cy + 1;
                            newtrian->b = points[by][bx];
                        }
                    }
                }else{// 如果位于下半球
                    if (0 == elementindex)// 如果索引为0（下半球索引为0的三角形是倒三角形）
                    {
                        newtrian->a = points[rowindex][0];
                        newtrian->b = points[rowindex + 1][0];
                        newtrian->c = points[rowindex][1];
                    }else{
                        int resverRowindex = private_calNumofRows(dl) - 1 - rowindex - 1;
                        int groupid = elementindex / (resverRowindex * 2 + 1);
                        if ((groupid % 2 == 0 && elementindex % 2 != 0) || (groupid % 2 != 0 && elementindex % 2 == 0))
                        {// 如果是正三角形
                            int cx = (elementindex + 1 - groupid) / 2 % private_calPointsoftheRow(dl, rowindex + 1);
                            int cy = rowindex + 1 ;
                            newtrian->c = points[cy][cx];
                            int ax = (elementindex + 1 - groupid) / 2 + groupid;
                            int ay = rowindex;
                            newtrian->a = points[ay][ax];
                            int bx = (elementindex + 1 - groupid) / 2 - 1;
                            int by = cy;
                            newtrian->b = points[by][bx];
                        }else{ // 如果是倒三角形
                            int cx = (elementindex - groupid) / 2 % private_calPointsoftheRow(dl, rowindex + 1);
                            int cy = rowindex + 1;
                            newtrian->c = points[cy][cx];
                            int ax = (elementindex - groupid) / 2 + groupid;
                            int ay = rowindex;
                            newtrian->a = points[ay][ax];
                            int bx = (ax + 1) % private_calPointsoftheRow(dl, rowindex);
                            int by = rowindex;
                            newtrian->b = points[by][bx];
                        }
                    }
                }
                private_initTriangleNormal(newtrian);
                triangles[rowindex].push_back(newtrian);
            }
        }
        // 手动插入最后一行三角形数据(倒三角)
        triangles.push_back(trians);
        maxRowIndex = private_calNumofRows(dl) - 1;
        newtrian = (TRIANGLE_F*)malloc(sizeof(TRIANGLE_F));
        newtrian->a = points[maxRowIndex - 1][0];
        newtrian->b = points[maxRowIndex][0];
        newtrian->c = points[maxRowIndex - 1][1];
        triangles[maxRowIndex - 1].push_back(newtrian);
        newtrian = (TRIANGLE_F*)malloc(sizeof(TRIANGLE_F));
        newtrian->a = points[maxRowIndex - 1][1];
        newtrian->b = points[maxRowIndex][0];
        newtrian->c = points[maxRowIndex - 1][2];
        triangles[maxRowIndex - 1].push_back(newtrian);
        newtrian = (TRIANGLE_F*)malloc(sizeof(TRIANGLE_F));
        newtrian->a = points[maxRowIndex - 1][2];
        newtrian->b = points[maxRowIndex][0];
        newtrian->c = points[maxRowIndex - 1][3];
        triangles[maxRowIndex - 1].push_back(newtrian);
        newtrian = (TRIANGLE_F*)malloc(sizeof(TRIANGLE_F));
        newtrian->a = points[maxRowIndex - 1][3];
        newtrian->b = points[maxRowIndex][0];
        newtrian->c = points[maxRowIndex - 1][0];
        triangles[maxRowIndex - 1].push_back(newtrian);
        // 初始化三角形的指针成员

    }
}

/*
* 析构
*/
CSphere::~CSphere()
{
    this->release();
}

/*
* 释放资源
*/
void CSphere::release()
{
    // 删除顶点数据
    for (int i = 0; (unsigned int)i < points.size(); ++i)
    {
        for (int j = 0; (unsigned int)j < points[i].size();++j)
        {
            free(points[i][j]);
            points[i][j] = nullptr;
        }
        points[i].clear();
    }
    points.clear();

    // 删除三角形数据
    for (int i = 0; (unsigned)i < triangles.size(); ++i)
    {
        for (int j = 0; (unsigned)j < triangles[i].size(); ++j)
        {
            free(points[i][j]);
            points[i][j] = nullptr;
        }
        points[i].clear();
    }
    triangles.clear();
}

/*
* 求冪(非法参数返回-1)
*/
int CSphere::private_power(const int &m, const int &n)
{
    if (n < 0)
        return -1;

    if (n == 0)
        return 1;

    if (n == 1)
        return m;

    return m*private_power(m, (n-1));
}

/*
* 计算当前复杂等级需要的行数
* 注意：南极点和北极点各算一行，赤道算一行，其他行数=private_calRowX(detailLevel)*2
*/
int CSphere::private_calNumofRows(const int &detailLevel)
{
    return (this->private_calRowX(detailLevel)*2 +3);
}

/*
* 使用迭代计算指定行的rowx
*/
int CSphere::private_calRowX(const int &detailLevel)
{
    if (detailLevel == 1)
        return 0;
    else
        return (this->private_calRowX(detailLevel - 1) * 2 + 1);
}

/*
* 计算指定复杂度下某行顶点数量
* @detailLevel:当前指定的复杂度
* @rowNumber:指定的行编号（从0开始）
*/
int CSphere::private_calPointsoftheRow(const int &detailLevel, const int &rowNumber)
{
    int result = (4 * private_power(2, (detailLevel - 1)) - (abs((private_calNumofRows(detailLevel) / 2) - rowNumber)) * 4);
    if (0 == result)
        return 1;
    else
        return result;
}

/*
* 计算指定复杂度下某行三角形数量
*/
int CSphere::private_calTriangleoftheRow(const int &dl, const int &rowNumber)
{
    if (rowNumber < (private_calNumofRows(dl) - 1)/2)   // 如果位于上半球
    {
        return (rowNumber * 2 + 1)*4;
    }else{                                              // 如果位于下半球
        int resverRowNumber = private_calNumofRows(dl) - 1 - rowNumber - 1;
        return (resverRowNumber * 2 + 1)*4;
    }
}

/*
* 取得vector<int>指定索引元素的迭代器（非安全）
*/
vector<POINT3D_F*>::iterator CSphere::private_getVectorofPoints_iterator(vector<POINT3D_F*> &pointsArray, int &index)
{
    return (pointsArray.begin() + index);
}

/*
* 取得vector<vector<POINT3D_F*>>指定索引数组的迭代器（非安全）
*/
vector<vector<POINT3D_F*>>::iterator CSphere::private_getVectorofVectorofPoints_iterator(
    vector<vector<POINT3D_F*>> &vectorArray, int &index
    )
{
    return (vectorArray.begin() + index);
}

/*
* 取得指定点的编号A邻居
* 注意：此为样板函数，其他函数可参考此函数
*/
POINT3D_F* CSphere::private_getNeighborA(int curDL, int rowindex, int elementindex)
{
    POINT3D_F *a = nullptr;
    //POINT3D_F *b = nullptr;
    //POINT3D_F *c = nullptr;
    //POINT3D_F *d = nullptr;
    //POINT3D_F *e = nullptr;
    //POINT3D_F *f = nullptr;
    if ( 0 == rowindex )
    {
        // 如果点属于第一行（北极点）,只有四个邻居a,b,c,d
        a = points[1][2];
        // b = points[1][3];
        // c = points[1][0];
        // d = points[1][1];
        // e = nullptr;
        // f = nullptr;
    }else if ( (private_calNumofRows(curDL)-1) == rowindex ){
        // 如果点属于最后一行（南极点）,只有四个邻居a,b,c,d
        a = points[private_calNumofRows(curDL) - 2][0];
        // b = points[preRowIndex][3];
        // c = points[preRowIndex][2];
        // d = points[preRowIndex][1];
        // e = nullptr;
        // f = nullptr;
    }else if (rowindex < private_calNumofRows(curDL)/2){
        // 如果点属于上半球（北极点和赤道之间）,那么有六个邻居abcdef
        if ( 0 == elementindex )
        {
            // 如果恰好是第一个元素
            a = points[rowindex - 1][private_calPointsoftheRow(curDL, rowindex - 1) - 1];
            // b = points[rowindex][private_calPointsoftheRow(curDL, rowindex) - 1];
            // c = points[rowindex + 1][0];
            // d = points[rowindex + 1][1];
            // e = points[rowindex][1];
            // f = points[rowindex - 1][0];
        }else{
            int groupid = 0;
            int y = 0;
            int x = 0;

            // a邻居索引
            if (elementindex % rowindex)
                groupid = (elementindex - elementindex % rowindex + rowindex) / rowindex;
            else
                groupid = elementindex / rowindex;
            y = rowindex - 1;
            x = elementindex - groupid;
            a = points[y][x];
            //// b邻居索引
            //y = rowindex;
            //x = elementindex - 1;
            //b = points[y][x];
            //// c邻居索引
            //if (elementindex % rowindex)
            //  groupid = (elementindex - elementindex % rowindex + rowindex) / rowindex;
            //else
            //  groupid = elementindex / rowindex;
            //y = rowindex + 1;
            //x = elementindex + groupid - 1;
            //c = points[y][x];
            // d邻居索引
            //if (elementindex % rowindex)
            //  groupid = (elementindex - elementindex % rowindex + rowindex) / rowindex;
            //else
            //  groupid = elementindex / rowindex;
            //y = rowindex + 1;
            //x = elementindex + groupid;
            //d = points[y][x];
            //// e邻居索引
            //y = rowindex;
            //x = (elementindex + 1) % private_calPointsoftheRow(curDL, rowindex);
            //e = points[y][x];
            //// f邻居索引
            //if (elementindex % rowindex)
            //  groupid = (elementindex - elementindex % rowindex + rowindex) / rowindex;
            //else
            //  groupid = elementindex / rowindex;
            //y = (rowindex - 1;
            //x = (elementindex - groupid + 1) % private_calPointsofthe(curDL, rowindex - 1);
            //f = points[y][x];
        }
    }else if (rowindex > private_calNumofRows(curDL)/2){
        // 如果是属于下半球（赤道和南极点之间）,那么有六个邻居abcdef
        if (0 == elementindex)
        {
            // 如果恰好是第一个元素
            a = points[rowindex - 1][0];
            // b = points[rowindex][private_calPointsoftheRow(curDL, rowindex) - 1];
            // c = points[rowindex + 1][private_calPointsoftheRow(curDL, rowindex+1) - 1];
            // d = points[rowindex + 1][0];
            // e = points[rowindex][1];
            // f = points[rowindex - 1][1];
        }else{
            int groupid = 0;
            int rowResverIndex = private_calNumofRows(curDL) - rowindex - 1;    // 当前行的反序索引
            int y = 0;
            int x = 0;
            // a邻居的索引
            if (elementindex % rowResverIndex)
                groupid = (elementindex - elementindex % rowResverIndex + rowResverIndex) / rowResverIndex;
            else
                groupid = elementindex / rowResverIndex;
            y = rowindex - 1;
            x = elementindex + groupid - 1;
            a = points[y][x];
            //// b邻居的索引
            //y = rowindex;
            //x = elementindex - 1;
            //b = points[y][x];
            //// c邻居的索引
            //if (elementindex % rowResverIndex)
            //  groupid = (elementindex - elementindex % rowResverIndex + rowResverIndex) / rowResverIndex - 1;
            //else
            //  groupid = elementindex / rowResverIndex ;
            //y = rowindex + 1;
            //x = elementindex - groupid;
            //c = points[y][x];
            //// d邻居的索引
            //if (elementindex % rowResverIndex)
            //  groupid = (elementindex - elementindex % rowResverIndex + rowResverIndex) / rowResverIndex;
            //else
            //  groupid = elementindex / rowResverIndex;
            //y = rowindex + 1;
            //x = (elementindex - groupid + 1) % private_calPointsoftheRow(curDL, y);
            //d = points[y][x];
            //// e邻居的索引
            //y = rowindex;
            //x = (elementindex + 1) % private_calPointsoftheRow(curDL, rowindex);
            //e = points[y][x];
            //// f邻居的索引
            //if (elementindex % rowResverIndex)
            //  groupid = (elementindex - elementindex % rowResverIndex + rowResverIndex) / rowResverIndex;
            //else
            //  groupid = elementindex / rowResverIndex;
            //y = rowindex - 1;
            //x = (elementindex + groupid) % private_calPointsoftheRow(curDL, rowindex - 1);
            //f = points[y][x];
        }
    }else if (private_calNumofRows(curDL)/2 == rowindex){
        if (0 == elementindex)
        {
            // 如果恰好是第一个元素
            a = points[rowindex - 1][private_calPointsoftheRow(curDL, rowindex - 1) - 1];
            // b = points[rowindex][private_calPointsoftheRow(curDL, rowindex) - 1] - 1;
            // c = points[rowindex + 1][0];
            // d = points[rowindex + 1][1];
            // e = points[rowindex][1];
            // f = points[rowindex - 1][0];
        }else{
            // 如果点属于赤道,那么有六个邻居abcdef
            int groupid = 0;
            int rowResverIndex = private_calNumofRows(curDL) - rowindex - 1;    // 当前行的反序索引
            int y = 0;
            int x = 0;
            // a邻居索引
            if (elementindex % rowindex)
                groupid = (elementindex - elementindex % rowindex + rowindex) / rowindex;
            else
                groupid = elementindex / rowindex;
            y = rowindex - 1;
            x = elementindex - groupid;
            a = points[y][x];
            //// b邻居索引
            //y = rowindex;
            //x = elementindex - 1;
            //b = points[y][x];
            //// c邻居索引
            //if (elementindex % rowResverIndex)
            //  groupid = (elementindex - elementindex % rowResverIndex + rowResverIndex) / rowResverIndex;
            //else
            //  groupid = elementindex / rowResverIndex;
            //y = rowindex + 1;
            //x = elementindex - groupid;
            //c = points[y][x];
            //// d邻居索引
            //if (elementindex % rowResverIndex)
            //  groupid = (elementindex - elementindex % rowResverIndex + rowResverIndex) / rowResverIndex;
            //else
            //  groupid = elementindex / rowResverIndex;
            //y = rowindex + 1;
            //x = (elementindex - groupid + 1) % private_calPointsoftheRow(curDL, y);
            //d = points[y][x];
            //// e邻居索引
            //y = rowindex;
            //x = (elementindex + 1) % private_calPointsoftheRow(curDL, rowindex);
            //e = points[y][x];
            //// f邻居索引
            //if (elementindex % rowindex)
            //  elementindex = (elementindex - elementindex % rowindex + rowindex) / rowindex;
            //else
            //  groupid = elementindex / rowindex;
            //y = rowindex - 1;
            //x = (elementindex - groupid + 1) % private_calPointsoftheRow(curDL, rowindex - 1);
            //f = points[y][x];
        }
    }

    return a;
    //return b;
    //return c;
    //return d;
    //return e;
    //return f;
}

/*
* 取得指定点的编号B邻居
*/
POINT3D_F* CSphere::private_getNeighborB(int curDL, int rowindex, int elementindex)
{
    POINT3D_F *b = nullptr;
    if (0 == rowindex)
    {
        // 如果点属于第一行（北极点）,只有四个邻居a,b,c,d
        b = points[1][3];
    }else if ((private_calNumofRows(curDL) - 1) == rowindex){
        // 如果点属于最后一行（南极点）,只有四个邻居a,b,c,d
        b = points[private_calNumofRows(curDL) - 2][3];
    }else if (rowindex < private_calNumofRows(curDL) / 2){
        // 如果点属于上半球（北极点和赤道之间）,那么有六个邻居abcdef
        if (0 == elementindex)
        {
            // 如果恰好是第一个元素
            b = points[rowindex][private_calPointsoftheRow(curDL, rowindex) - 1];
        }else{
            int groupid = 0;
            int y = 0;
            int x = 0;

            // b邻居索引
            y = rowindex;
            x = elementindex - 1;
            b = points[y][x];
        }
    }else if (rowindex > private_calNumofRows(curDL) / 2){
        // 如果是属于下半球（赤道和南极点之间）,那么有六个邻居abcdef
        if (0 == elementindex)
            // 如果恰好是第一个元素
            b = points[rowindex][private_calPointsoftheRow(curDL, rowindex) - 1];
        else
            b = points[rowindex][elementindex - 1];
    }else if (private_calNumofRows(curDL) / 2 == rowindex){
        if (0 == elementindex)
            // 如果恰好是第一个元素
            b = points[rowindex][private_calPointsoftheRow(curDL, rowindex) - 1] - 1;
        else
            b = points[rowindex][elementindex - 1];
    }

    return b;
}

/*
* 取得指定点的编号C邻居
*/
POINT3D_F* CSphere::private_getNeighborC(int curDL, int rowindex, int elementindex)
{
    POINT3D_F *c = nullptr;
    if (0 == rowindex)
    {
        // 如果点属于第一行（北极点）,只有四个邻居a,b,c,d
        c = points[1][0];
    }else if ((private_calNumofRows(curDL) - 1) == rowindex){
        // 如果点属于最后一行（南极点）,只有四个邻居a,b,c,d
        c = points[private_calNumofRows(curDL) - 2][2];
    }else if (rowindex < private_calNumofRows(curDL) / 2){
        // 如果点属于上半球（北极点和赤道之间）,那么有六个邻居abcdef
        if (0 == elementindex)
        {
            c = points[rowindex + 1][0];
        }else{
            int groupid = 0;
            // c邻居索引
            if (elementindex % rowindex)
                groupid = (elementindex - elementindex % rowindex + rowindex) / rowindex;
            else
                groupid = elementindex / rowindex;
            c = points[rowindex + 1][elementindex + groupid - 1];
        }
    }else if (rowindex > private_calNumofRows(curDL) / 2){
        // 如果是属于下半球（赤道和南极点之间）,那么有六个邻居abcdef
        if (0 == elementindex)
        {
            // 如果恰好是第一个元素
            c = points[rowindex + 1][private_calPointsoftheRow(curDL, rowindex+1) - 1];
        }else{
            int groupid = 0;
            int rowResverIndex = private_calNumofRows(curDL) - rowindex - 1;    // 当前行的反序索引
            // c邻居的索引
            if (elementindex % rowResverIndex)
                groupid = (elementindex - elementindex % rowResverIndex + rowResverIndex) / rowResverIndex;
            else
                groupid = elementindex / rowResverIndex ;
            c = points[rowindex + 1][elementindex - groupid];
        }
    }else if (private_calNumofRows(curDL) / 2 == rowindex){
        if (0 == elementindex)
        {
            // 如果恰好是第一个元素
            c = points[rowindex + 1][0];
        }else{
            // 如果点属于赤道,那么有六个邻居abcdef
            int groupid = 0;
            int rowResverIndex = private_calNumofRows(curDL) - rowindex - 1;    // 当前行的反序索引
            // c邻居索引
            if (elementindex % rowResverIndex)
                groupid = (elementindex - elementindex % rowResverIndex + rowResverIndex) / rowResverIndex;
            else
                groupid = elementindex / rowResverIndex;
            c = points[rowindex + 1][elementindex - groupid];
        }
    }

    return c;
}

/*
* 取得指定点的编号D邻居
*/
POINT3D_F* CSphere::private_getNeighborD(int curDL, int rowindex, int elementindex)
{
    POINT3D_F *d = nullptr;
    if (0 == rowindex)
    {
        // 如果点属于第一行（北极点）,只有四个邻居a,b,c,d
        d = points[1][1];
    }else if ((private_calNumofRows(curDL) - 1) == rowindex){
        // 如果点属于最后一行（南极点）,只有四个邻居a,b,c,d
        d = points[private_calNumofRows(curDL) - 2][1];
    }else if (rowindex < private_calNumofRows(curDL) / 2){
        // 如果点属于上半球（北极点和赤道之间）,那么有六个邻居abcdef
        if (0 == elementindex)
        {
            d = points[rowindex + 1][1];
        }else{
            int groupid = 0;
            // d邻居索引
            if (elementindex % rowindex)
                groupid = (elementindex - elementindex % rowindex + rowindex) / rowindex;
            else
                groupid = elementindex / rowindex;
            d = points[rowindex + 1][elementindex + groupid];
        }
    }else if (rowindex > private_calNumofRows(curDL) / 2){
        // 如果是属于下半球（赤道和南极点之间）,那么有六个邻居abcdef
        if (0 == elementindex)
        {
            // 如果恰好是第一个元素
            d = points[rowindex + 1][0];
        }else{
            int groupid = 0;
            int rowResverIndex = private_calNumofRows(curDL) - rowindex - 1;    // 当前行的反序索引
            int y = 0;
            int x = 0;

            // d邻居的索引
            if (elementindex % rowResverIndex)
                groupid = (elementindex - elementindex % rowResverIndex + rowResverIndex) / rowResverIndex;
            else
                groupid = elementindex / rowResverIndex;
            y = rowindex + 1;
            x = (elementindex - groupid + 1) % private_calPointsoftheRow(curDL, y);
            d = points[y][x];
        }
    }else if (private_calNumofRows(curDL) / 2 == rowindex){
        if (0 == elementindex)
        {
            // 如果恰好是第一个元素
            d = points[rowindex + 1][1];
        }else{
            // 如果点属于赤道,那么有六个邻居abcdef
            int groupid = 0;
            int rowResverIndex = private_calNumofRows(curDL) - rowindex - 1;    // 当前行的反序索引
            int y = 0;
            int x = 0;

            // d邻居索引
            if (elementindex % rowResverIndex)
                groupid = (elementindex - elementindex % rowResverIndex + rowResverIndex) / rowResverIndex;
            else
                groupid = elementindex / rowResverIndex;
            y = rowindex + 1;
            x = (elementindex - groupid + 1) % private_calPointsoftheRow(curDL, y);
            d = points[y][x];
        }
    }

    return d;
}

/*
* 取得指定点的编号E邻居
*/
POINT3D_F* CSphere::private_getNeighborE(int curDL, int rowindex, int elementindex)
{
    POINT3D_F *e = nullptr;
    if (0 == rowindex)
    {
        // 如果点属于第一行（北极点）,只有四个邻居a,b,c,d
        e = nullptr;
    }else if ((private_calNumofRows(curDL) - 1) == rowindex){
        // 如果点属于最后一行（南极点）,只有四个邻居a,b,c,d
        e = nullptr;
    }else if (rowindex < private_calNumofRows(curDL) / 2){
        // 如果点属于上半球（北极点和赤道之间）,那么有六个邻居abcdef
        if (0 == elementindex)
        {
            e = points[rowindex][1];
        }else{
            int groupid = 0;
            int y = 0;
            int x = 0;

            // e邻居索引
            y = rowindex;
            x = (elementindex + 1) % private_calPointsoftheRow(curDL, rowindex);
            e = points[y][x];
        }
    }else if (rowindex > private_calNumofRows(curDL) / 2){
        // 如果是属于下半球（赤道和南极点之间）,那么有六个邻居abcdef
        if (0 == elementindex)
        {
            // 如果恰好是第一个元素
            e = points[rowindex][1];
        }else{
            int groupid = 0;
            int rowResverIndex = private_calNumofRows(curDL) - rowindex - 1;    // 当前行的反序索引
            int y = 0;
            int x = 0;

            // e邻居的索引
            y = rowindex;
            x = (elementindex + 1) % private_calPointsoftheRow(curDL, rowindex);
            e = points[y][x];
        }
    }else if (private_calNumofRows(curDL) / 2 == rowindex){
        if (0 == elementindex)
        {
            // 如果恰好是第一个元素
            e = points[rowindex][1];
        }else{
            // 如果点属于赤道,那么有六个邻居abcdef
            int groupid = 0;
            int rowResverIndex = private_calNumofRows(curDL) - rowindex - 1;    // 当前行的反序索引
            int y = 0;
            int x = 0;

            // e邻居索引
            y = rowindex;
            x = (elementindex + 1) % private_calPointsoftheRow(curDL, rowindex);
            e = points[y][x];
        }
    }

    return e;
}

/*
* 取得指定点的编号F邻居
*/
POINT3D_F* CSphere::private_getNeighborF(int curDL, int rowindex, int elementindex)
{
    POINT3D_F *f = nullptr;
    if (0 == rowindex)
    {
        // 如果点属于第一行（北极点）,只有四个邻居a,b,c,d
        f = nullptr;
    }else if ((private_calNumofRows(curDL) - 1) == rowindex){
        // 如果点属于最后一行（南极点）,只有四个邻居a,b,c,d
        f = nullptr;
    }else if (rowindex < private_calNumofRows(curDL) / 2){
        // 如果点属于上半球（北极点和赤道之间）,那么有六个邻居abcdef
        if (0 == elementindex)
        {
            f = points[rowindex - 1][0];
        }else{
            int groupid = 0;
            int y = 0;
            int x = 0;

            // f邻居索引
            if (elementindex % rowindex)
                groupid = (elementindex - elementindex % rowindex + rowindex) / rowindex;
            else
                groupid = elementindex / rowindex;
            y = rowindex - 1;
            x = (elementindex - groupid + 1) % private_calPointsoftheRow(curDL, rowindex - 1);
            f = points[y][x];
        }
    }else if (rowindex > private_calNumofRows(curDL) / 2){
        // 如果是属于下半球（赤道和南极点之间）,那么有六个邻居abcdef
        if (0 == elementindex)
        {
            // 如果恰好是第一个元素
            f = points[rowindex - 1][1];
        }else{
            int groupid = 0;
            int rowResverIndex = private_calNumofRows(curDL) - rowindex - 1;    // 当前行的反序索引
            int y = 0;
            int x = 0;

            // f邻居的索引
            if (elementindex % rowResverIndex)
                groupid = (elementindex - elementindex % rowResverIndex + rowResverIndex) / rowResverIndex;
            else
                groupid = elementindex / rowResverIndex;
            y = rowindex - 1;
            x = (elementindex + groupid) % private_calPointsoftheRow(curDL, rowindex - 1);
            f = points[y][x];
        }
    }else if (private_calNumofRows(curDL) / 2 == rowindex){
        if (0 == elementindex)
        {
            // 如果恰好是第一个元素
            f = points[rowindex - 1][0];
        }else{
            // 如果点属于赤道,那么有六个邻居abcdef
            int groupid = 0;
            int rowResverIndex = private_calNumofRows(curDL) - rowindex - 1;    // 当前行的反序索引
            int y = 0;
            int x = 0;

            // f邻居索引
            if (elementindex % rowindex)
                elementindex = (elementindex - elementindex % rowindex + rowindex) / rowindex;
            else
                groupid = elementindex / rowindex;
            y = rowindex - 1;
            x = (elementindex - groupid + 1) % private_calPointsoftheRow(curDL, rowindex - 1);
            f = points[y][x];
        }
    }

    return f;
}

/*
* 计算两个向量的中间向量，并返回中间向量的单位向量
*/
VECTOR_F CSphere::private_calMidVector(const VECTOR_F &v1, const VECTOR_F &v2)
{
    VECTOR_F result(0.0f, 0.0f, 0.0f);
    float delta[3] = { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z };
    result.x = v2.x + delta[0] / 2.0f;
    result.y = v2.y + delta[1] / 2.0f;
    result.z = v2.z + delta[2] / 2.0f;
    // 单位化
    float len = sqrtf(result.x*result.x + result.y*result.y + result.z*result.z);
    result.x /= len;
    result.y /= len;
    result.z /= len;

    return result;
}

/*
* 根据三角形三个顶点计算三角形法线
*/
void CSphere::private_initTriangleNormal(TRIANGLE_F *triangle)
{
    // 首先计算出圆心到三角形中心的向量
    triangle->normal[0] = (triangle->a->x + triangle->b->x + triangle->c->x) / 3.0f;
    triangle->normal[1] = (triangle->a->y + triangle->b->y + triangle->c->y) / 3.0f;
    triangle->normal[2] = (triangle->a->z + triangle->b->z + triangle->c->z) / 3.0f;
    // 单位化
    float len = sqrtf( triangle->normal[0] * triangle->normal[0] + triangle->normal[1] * triangle->normal[1] + triangle->normal[2] * triangle->normal[2] );
    triangle->normal[0] /= len;
    triangle->normal[1] /= len;
    triangle->normal[2] /= len;
}
