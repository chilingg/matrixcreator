#include <QDebug>

#include "matrixmodel.h"

MatrixModel::MatrixModel()
{
    for(int i = 0; i < WORLDSIZE; ++i)
    {
        for(int j = 0; j < WORLDSIZE; ++j)
        {
            model[i][j] = 0;
        }
    }//初始化数组为0
}

int MatrixModel::getModelValue(int x, int y)
{
    if(x > WORLDSIZE || y > WORLDSIZE)
        qDebug() << "Over range!" << x << y;

    return (model[x][y]);

}

void MatrixModel::updateModel()
{
    //更新模型，奇变偶 偶变奇 <==Test
    for(int i = 0; i < WORLDSIZE; ++i)
    {
        for(int j = 0; j < WORLDSIZE; ++j)
        {
            int aroundValue = getAroundValue(i, j);
            aroundValue += model[i][j] ? 10 : 0;//aroundValue大于8则当前状态为生
            switch (aroundValue)
            {
            case 0:
            case 1:
            case 2:
                break;
            case 3:
                model[i][j] = 1;
                break;
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
                break;
            case 10:
            case 11:
                model[i][j] = 0;
                break;
            case 12:
            case 13:
                break;
            case 14:
            case 15:
            case 16:
            case 17:
            case 18:
                model[i][j] = 0;
                break;
            default:
                qDebug() << "AroundValue Over range!" << aroundValue ;
                break;
            }
        }
    }
}

void MatrixModel::changeModelValue(int x, int y)
{
    if(x > WORLDSIZE || y > WORLDSIZE)
        qDebug() << "Over range!" << x << y;

    model[x][y] = !model[x][y];
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

    int aroundValue = model[around_1X][around_1Y]
            + model[around_2X][around_2Y]
            + model[around_3X][around_3Y]
            + model[around_4X][around_4Y]
            + model[around_7X][around_7Y]
            + model[around_8X][around_8Y]
            + model[around_9X][around_9Y]
            + model[around_6X][around_6Y];

    return aroundValue;
}
