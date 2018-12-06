#include <QDebug>
#include <QtConcurrent>
#include "matrixmodel.h"

MatrixModel::MatrixModel():
    currentModel(new int[WORLDSIZE][WORLDSIZE]),
    tempModel(new int[WORLDSIZE][WORLDSIZE])
{
    clearAllModel();
    updateLine = 0;

}

MatrixModel::~MatrixModel()
{
    delete []currentModel;
    delete []tempModel;
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

    for(size_t i = 0; i < WORLDSIZE; ++i)
    {
        for(size_t j = 0; j < WORLDSIZE; ++j)
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

void MatrixModel::changeModelValue(size_t x, size_t y)
{
    if(x > WORLDSIZE || y > WORLDSIZE)
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

void MatrixModel::transferModelThread()
{
    beginUpdate();

    QFuture<void> future[THREADS];
    for(size_t i = 0; i < THREADS; ++i)
    {
        future[i] = QtConcurrent::run(this, &MatrixModel::startTransfer);
    }
    for(size_t i = 0; i < THREADS; ++i)
    {
        future[i].waitForFinished();
    }
    if(debug != WORLDSIZE)
        qDebug() << "Thread runs:" << WORLDSIZE - debug;
    debug = 0;

    //把current指向新模型，temp指向旧模型
    int(* tempP)[WORLDSIZE] = currentModel;
    currentModel = tempModel;
    tempModel = tempP;
}

bool MatrixModel::updateStatus() const
{
    return currentStatus;
}

void MatrixModel::beginUpdate()
{
    currentStatus = true;
}

size_t MatrixModel::getUpdateLine()
{
    QMutexLocker locker(&lineMutex);

    //qDebug() << "In getUpdateLine..." << updateLine;

    if(!currentStatus)
        return 0;

    if(updateLine >= WORLDSIZE)
    {
        updateLine = 0;
        currentStatus = false;
        return 0;
    }

    ++debug;
    return updateLine++;
}

const int (*MatrixModel::getModel())[WORLDSIZE]
{
    return currentModel;
}

int MatrixModel::getAroundValue(size_t x, size_t y)
{
    size_t around_1X = x != 0 ? x - 1 : WORLDSIZE - 1;
    size_t around_1Y = y != 0 ? y - 1 : WORLDSIZE - 1;

    size_t around_2X = x;
    size_t around_2Y = y != 0 ? y - 1 : WORLDSIZE - 1;

    size_t around_3X = x != WORLDSIZE - 1 ? x + 1 : 0;
    size_t around_3Y = y != 0 ? y - 1 : WORLDSIZE - 1;

    size_t around_4X = x != 0 ? x - 1 : WORLDSIZE - 1;
    size_t around_4Y = y;

    size_t around_6X = x != WORLDSIZE - 1 ? x + 1 : 0;
    size_t around_6Y = y;

    size_t around_7X = x != 0 ? x - 1 : WORLDSIZE - 1;
    size_t around_7Y = y != WORLDSIZE - 1 ? y + 1 : 0;

    size_t around_8X = x;
    size_t around_8Y = y != WORLDSIZE - 1 ? y + 1 : 0;

    size_t around_9X = x != WORLDSIZE - 1 ? x + 1 : 0;
    size_t around_9Y = y != WORLDSIZE - 1 ? y + 1 : 0;

    int aroundValue = currentModel[around_1X][around_1Y]
            + currentModel[around_2X][around_2Y]
            + currentModel[around_3X][around_3Y]
            + currentModel[around_4X][around_4Y]
            + currentModel[around_7X][around_7Y]
            + currentModel[around_8X][around_8Y]
            + currentModel[around_9X][around_9Y]
            + currentModel[around_6X][around_6Y];

    if(aroundValue > 8)
    {
        qDebug() << "aroundValue overflow!" << x << y;

        if(around_1X >= WORLDSIZE)
            qDebug() << around_1X << "around_1X";
        if(around_1Y > WORLDSIZE)
            qDebug() << around_1Y << "around_1Y";

        if(around_2X >= WORLDSIZE)
            qDebug() << around_2X << "around_2X";
        if(around_2Y > WORLDSIZE)
            qDebug() << around_2Y << "around_2Y";

        if(around_3X >= WORLDSIZE)
            qDebug() << around_3X << "around_3X";
        if(around_3Y >= WORLDSIZE)
            qDebug() << around_3Y << "around_3Y";

        if(around_4X >= WORLDSIZE)
            qDebug() << around_4X << "around_4X";
        if(around_4Y >= WORLDSIZE)
            qDebug() << around_4Y << "around_4Y";

        if(around_6X >= WORLDSIZE)
            qDebug() << around_6X << "around_6X";
        if(around_6Y >= WORLDSIZE)
            qDebug() << around_6Y << "around_6Y";

        if(around_7X >= WORLDSIZE)
            qDebug() << around_7X << "around_7X";
        if(around_7Y >= WORLDSIZE)
            qDebug() << around_7Y << "around_7Y";

        if(around_8X >= WORLDSIZE)
            qDebug() << around_8X << "around_8X";
        if(around_8Y >= WORLDSIZE)
            qDebug() << around_8Y << "around_8Y";

        if(around_9X > WORLDSIZE)
            qDebug() << around_9X << "around_9X";
        if(around_9Y > WORLDSIZE)
            qDebug() << around_9Y << "around_9Y";
    }

    return aroundValue;
}

void MatrixModel::changLineAroundValue(size_t line)
{
    //QMutexLocker locker(&changeMutex);
    //qDebug() << " 1";

    for(size_t y = 0; y < WORLDSIZE; ++y)
    {
        if(currentModel[line][y] == 0)
            continue;

        if(line >= WORLDSIZE || y > WORLDSIZE)
            qDebug() << line << y << "changeAroundValue error";

        size_t around_1X = line != 0 ? line - 1 : WORLDSIZE - 1;
        size_t around_1Y = y != 0 ? y - 1 : WORLDSIZE - 1;

        size_t around_2X = line;
        size_t around_2Y = y != 0 ? y - 1 : WORLDSIZE - 1;

        size_t around_3X = line != WORLDSIZE - 1 ? line + 1 : 0;
        size_t around_3Y = y != 0 ? y - 1 : WORLDSIZE - 1;

        size_t around_4X = line != 0 ? line - 1 : WORLDSIZE - 1;
        size_t around_4Y = y;

        size_t around_6X = line != WORLDSIZE - 1 ? line + 1 : 0;
        size_t around_6Y = y;

        size_t around_7X = line != 0 ? line - 1 : WORLDSIZE - 1;
        size_t around_7Y = y != WORLDSIZE - 1 ? y + 1 : 0;

        size_t around_8X = line;
        size_t around_8Y = y != WORLDSIZE - 1 ? y + 1 : 0;

        size_t around_9X = line != WORLDSIZE - 1 ? line + 1 : 0;
        size_t around_9Y = y != WORLDSIZE - 1 ? y + 1 : 0;

        changeMutex.lock();
        int t = tempModel[line][y];
        tempModel[around_1X][around_1Y] += 1;
        tempModel[around_2X][around_2Y] += 1;
        tempModel[around_3X][around_3Y] += 1;
        tempModel[around_4X][around_4Y] += 1;
        tempModel[line][y] += 10; //值大于等于10则表示对应current有值
        tempModel[around_6X][around_6Y] += 1;
        tempModel[around_7X][around_7Y] += 1;
        tempModel[around_8X][around_8Y] += 1;
        tempModel[around_9X][around_9Y] += 1;

        if(tempModel[line][y] > 18)
            qDebug() << "Unclear!" << line << y << tempModel[line][y] << t;
        changeMutex.unlock();
    }
    //qDebug() << "1 ";
}

void MatrixModel::transferModelLine(size_t line)
{
    //qDebug() << updateLine << "UpdateLine";

    for(size_t j = 0; j < WORLDSIZE; ++j)
    {
        int aroundValue = getAroundValue(line, j);
        aroundValue += currentModel[line][j] ? 10 : 0;//aroundValue大于8则当前状态为生

        //计算出的模型存在tempModel中，避免影响正在进行的getAroundValue计算
        tempModel[line][j] = currentModel[line][j];
        switch (aroundValue)
        {
        case 0:
        case 1:
        case 2:
            break;
        case 3:
            tempModel[line][j] = 1;
            break;
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
            break;
        case 10:
        case 11:
            tempModel[line][j] = 0;
            break;
        case 12:
        case 13:
            break;
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
            tempModel[line][j] = 0;
            break;
        default:
            qDebug() << "AroundValue Over range!" << aroundValue ;
            break;
        }
    }
    //qDebug() << line << "In updating...";
}

void MatrixModel::startTransfer()
{
    size_t line = getUpdateLine();

    while (updateStatus() || line != 0)
    {
        //qDebug() << "In thread run..." << line;

        if(line >= WORLDSIZE)
            qDebug() << "Overflow line" << line;

        transferModelLine(line);
        line = getUpdateLine();
    }
}

void MatrixModel::calculusModelThread()
{
    beginUpdate();
    QFuture<void> future[THREADS];
    for(size_t i = 0; i < THREADS; ++i)
    {
        future[i] = QtConcurrent::run(this, &MatrixModel::startCalculus1);
    }
    for(size_t i = 0; i < THREADS; ++i)
    {
        future[i].waitForFinished();
    }
    if(debug != WORLDSIZE)
        qDebug() << "Thread runs:" << WORLDSIZE - debug;
    debug = 0;

    beginUpdate();
    for(size_t i = 0; i < THREADS; ++i)
    {
        future[i] = QtConcurrent::run(this, &MatrixModel::startCalculus2);
    }
    for(size_t i = 0; i < THREADS; ++i)
    {
        future[i].waitForFinished();
    }
    if(debug != WORLDSIZE)
        qDebug() << "Thread2 runs:" << WORLDSIZE - debug;
    debug = 0;
}

void MatrixModel::calculusModelLine(size_t line)
{
    QMutex tempMutex;
    QMutexLocker locker(&tempMutex);

    for(int y = 0; y < WORLDSIZE; ++y)
    {
        //if(tempModel[line][y] == 0 && currentModel[line][y] == 0)
            //continue;

        switch (tempModel[line][y])
        {
        case 0:
        case 1:
        case 2:
            break;
        case 3:
            currentModel[line][y] = 1;
            break;
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
            break;
        case 10:
        case 11:
            currentModel[line][y] = 0;
            break;
        case 12:
        case 13:
            break;
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
            currentModel[line][y] = 0;
            break;

        default:
            qDebug() << "Value Error!" << line << y << tempModel[line][y];
            currentModel[line][y] = 2;
            break;
        }

        tempModel[line][y] = 0;
    }
}

void MatrixModel::startCalculus1()
{
    size_t line = getUpdateLine();
    //qDebug() << "In startCalculus1..." << line;

    while (updateStatus() || line != 0)
    {
        changLineAroundValue(line);
        line = getUpdateLine();
    }
    if(line != 0)
        qDebug() << "Line Error" << line;
}

void MatrixModel::startCalculus2()
{
    size_t line = getUpdateLine();
    //qDebug() << "In startCalculus2..." << line;

    while (updateStatus() || line != 0)
    {
        calculusModelLine(line);
        line = getUpdateLine();
    }
}

void MatrixModel::testModelThread()
{
    beginUpdate();

    QFuture<void> future[THREADS];
    for(size_t i = 0; i < THREADS; ++i)
    {
        future[i] = QtConcurrent::run(this, &MatrixModel::startTest);
    }
    for(size_t i = 0; i < THREADS; ++i)
    {
        future[i].waitForFinished();
    }
    if(debug != WORLDSIZE)
        qDebug() << "Thread runs:" << WORLDSIZE - debug;
    debug = 0;
}

void MatrixModel::testModelLine(size_t line)
{
    for(size_t j = 0; j < WORLDSIZE; ++j)
    {
        changeModelValue(line, j);
    }
}

void MatrixModel::startTest()
{
    size_t line = getUpdateLine();

    while (updateStatus() || line != 0)
    {
        testModelLine(line);
        line = getUpdateLine();
    }
}
