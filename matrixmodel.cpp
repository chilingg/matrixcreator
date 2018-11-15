#include <QDebug>
#include "matrixmodel.h"

MatrixModel::MatrixModel():
    currentModel(new int[WORLDSIZE][WORLDSIZE]),
    tempModel(new int[WORLDSIZE][WORLDSIZE])
{
    clearAllModel();
}

MatrixModel::~MatrixModel()
{
    delete []currentModel;
    delete []tempModel;

    currentModel = 0;
    tempModel = 0;
}

int MatrixModel::getModelValue(int x, int y)
{
    if(x > WORLDSIZE || y > WORLDSIZE)
    {
        qDebug() << "Over range!(get)" << x << y;
        return 0;
    }

    return (currentModel[x][y]);

}

void MatrixModel::updateModel()
{
    //qDebug() << "M";

    for(int i = 0; i < WORLDSIZE; ++i)
    {
        for(int j = 0; j < WORLDSIZE; ++j)
        {
            int aroundValue = getAroundValue(i, j);
            aroundValue += currentModel[i][j] ? 10 : 0;//aroundValue大于8则当前状态为生

            //计算出的模型存在tempModel中，避免影响正在进行的getAroundValue计算
            tempModel[i][j] = currentModel[i][j];
            switch (aroundValue)
            {
            case 0:
            case 1:
            case 2:
                break;
            case 3:
                tempModel[i][j] = 1;
                break;
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
                break;
            case 10:
            case 11:
                tempModel[i][j] = 0;
                break;
            case 12:
            case 13:
                break;
            case 14:
            case 15:
            case 16:
            case 17:
            case 18:
                tempModel[i][j] = 0;
                break;
            default:
                qDebug() << "AroundValue Over range!" << aroundValue ;
                break;
            }
        }
    }
    //把current指向新模型，temp指向旧模型
    int(* tempP)[WORLDSIZE] = currentModel;
    currentModel = tempModel;
    tempModel = tempP;
}

void MatrixModel::changeModelValue(int x, int y)
{
    if(x > WORLDSIZE || y > WORLDSIZE || x < 0 || y < 0)
    {
        qDebug() << "Over range!(change)" << x << y;
        return;
    }

    currentModel[x][y] = !currentModel[x][y];
}

void MatrixModel::clearModel(int x, int y, int widht, int height)
{
    //qDebug() << x << y << widht << height;
    if(x < 0 || y < 0 || x + widht > WORLDSIZE || y + height > WORLDSIZE)
    {
        //qDebug() << "Over range!(clear)" << x << y << widht << height;
        return;
    }

    for(int i = x; i < widht + x; ++i)
    {
        for(int j = y; j < height + y; ++j)
        {
            currentModel[i][j] = 0;
        }
    }
}

void MatrixModel::clearAllModel()
{
    //qsrand(32);//随机测试
    for(int i = 0; i < WORLDSIZE; ++i)
    {
        for(int j = 0; j < WORLDSIZE; ++j)
        {
            //currentModel[i][j] = qrand() % 16 ? 0 : 1;
            currentModel[i][j] = 0;
            tempModel[i][j] = 0;
        }
    }//初始化数组为0
}

int MatrixModel::getAroundValue(int x, int y)
{
    int around_1X = x != 0 ? x - 1 : WORLDSIZE - 1;
    int around_1Y = y != 0 ? y - 1 : WORLDSIZE - 1;

    int around_2X = x;
    int around_2Y = y != 0 ? y - 1 : WORLDSIZE - 1;

    int around_3X = x != WORLDSIZE - 1 ? x + 1 : 0;
    int around_3Y = y != 0 ? y - 1 : WORLDSIZE - 1;

    int around_4X = x != 0 ? x - 1 : WORLDSIZE - 1;
    int around_4Y = y;

    int around_6X = x != WORLDSIZE - 1 ? x + 1 : 0;
    int around_6Y = y;

    int around_7X = x != 0 ? x - 1 : WORLDSIZE - 1;
    int around_7Y = y != WORLDSIZE - 1 ? y + 1 : 0;

    int around_8X = x;
    int around_8Y = y != WORLDSIZE - 1 ? y + 1 : 0;

    int around_9X = x != WORLDSIZE - 1 ? x + 1 : 0;
    int around_9Y = y != WORLDSIZE - 1 ? y + 1 : 0;

    int aroundValue = currentModel[around_1X][around_1Y]
            + currentModel[around_2X][around_2Y]
            + currentModel[around_3X][around_3Y]
            + currentModel[around_4X][around_4Y]
            + currentModel[around_7X][around_7Y]
            + currentModel[around_8X][around_8Y]
            + currentModel[around_9X][around_9Y]
            + currentModel[around_6X][around_6Y];

    return aroundValue;
}
