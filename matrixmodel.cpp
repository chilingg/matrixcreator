#include "matrixmodel.h"

MatrixModel::MatrixModel(unsigned size, ModelPattern pattern):
    THREADS(std::thread::hardware_concurrency()),
    future(THREADS),	//获取cpu核数
    currentModel(new int*[size]),
    modelPattern(EmptyPattern),
    modelSize(size),
    updateStatus(false),
    updateLine(0)
{
    for(size_t i =0; i < modelSize; ++i)
    {
        currentModel[i] = new int[modelSize];
    }
    clearAllUnit();

    modelPattern = switchModel(pattern);
}

MatrixModel::~MatrixModel()
{
    switchModel(EmptyPattern);
    for(size_t i =0; i < modelSize; ++i)
    {
        delete [] currentModel[i];
    }
    delete [] currentModel;
}

void MatrixModel::clearUnit(MatrixSize x, MatrixSize y, MatrixSize widht, MatrixSize height)
{
#ifndef M_NO_DEBUG
    if(x+widht-1 > modelSize || y+height-1 > modelSize)
        qDebug() << "Log in" << __FILE__ << ":" << __FUNCTION__ << " line: " << __LINE__
                 << "error: Index out of range!";
#endif

    for(MatrixSize i = 0; i < widht; ++i)
    {
        for(MatrixSize j = 0; j < height; ++j)
        {
            currentModel[x+i][y+j] = 0;
        }
    }
}

void MatrixModel::clearAllUnit()
{
    clearUnit(0, 0, modelSize, modelSize);
}

MatrixModel::ModelPattern MatrixModel::switchModel(MatrixModel::ModelPattern after)
{
    //结束之前的模式
    switch (modelPattern) {
    case LifeGameT:
        for(size_t i =0; i < modelSize; ++i)
        {
            delete [] tTempModel[i];
        }
        delete [] tTempModel;
        break;
    case LifeGame:
        for(size_t i =0; i < modelSize; ++i)
        {
            delete [] cTempModel[i];
        }
        delete [] cTempModel;
        break;
    default:
        break;
    }

    //开始新的模式
    switch (after) {
    case LifeGameT:
        tTempModel = new int*[modelSize];
        for(size_t i =0; i < modelSize; ++i)
        {
            tTempModel[i] = new int[modelSize];
        }
        for(MatrixSize i = 0; i < modelSize; ++i)
        {
            for(MatrixSize j = 0; j < modelSize; ++j)
            {
                tTempModel[i][j] = 0;
            }
        }
        updateModel = &MatrixModel::LFTransferModelThread;
        break;
    case LifeGame:
        cTempModel = new int*[modelSize];
        for(size_t i =0; i < modelSize; ++i)
        {
            cTempModel[i] = new int[modelSize];
        }
        for(MatrixSize i = 0; i < modelSize; ++i)
        {
            for(MatrixSize j = 0; j < modelSize; ++j)
            {
                cTempModel[i][j] = 0;
            }
        }
        updateModel = &MatrixModel::LFCalculusModelThread;
        break;
    default:
        updateModel = nullptr;
        break;
    }

    return after;
}

MatrixSize MatrixModel::getUpdateLine()
{
    //QMutexLocker创建时锁定资源，析构时解锁后其它线程才能进入
    QMutexLocker locker(&lineMutex);

    if(!updateStatus)
        return 0;

    if(updateLine >= modelSize)
    {
        updateLine = 0;
        updateStatus = false;
        return 0;
    }

    return updateLine++;
}

void MatrixModel::LFTransferModelThread()
{
    beginUpdate();

    for(MatrixSize i = 0; i < THREADS; ++i)
    {
        future[i] = QtConcurrent::run(this, &MatrixModel::startTransfer);
    }
    for(MatrixSize i = 0; i < THREADS; ++i)
    {
        future[i].waitForFinished();//等待所有线程结束
    }
#ifndef M_NO_DEBUG
    if(debug != modelSize)
        qDebug() << "Log in" << __FILE__ << ":" << __FUNCTION__ << " line: " << __LINE__
                 << "Thread runs:" << modelSize - debug;
    debug = 0;
#endif

    //把current指向新模型，temp指向旧模型
    int **temp = currentModel;
    currentModel = tTempModel;
    tTempModel = temp;
}

void MatrixModel::transferModelLine(MatrixSize line)
{
#ifndef M_NO_DEBUG
    changeMutex.lock();
    ++debug;
    changeMutex.unlock();
#endif

    for(MatrixSize j = 0; j < modelSize; ++j)
    {
        int aroundValue = getAroundValue(line, j);

        //计算出的模型存在tempModel中，避免影响正在进行的getAroundValue计算
        tTempModel[line][j] = currentModel[line][j];
        switch (aroundValue)
        {
        case 0:
        case 1:
        case 2:
            break;
        case 3:
            tTempModel[line][j] = 1;
            break;
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
            break;
        case 10:
        case 11:
            tTempModel[line][j] = 0;
            break;
        case 12:
        case 13:
            break;
        case 14:
        case 15:
        case 16:
        case 17:
        case 18:
            tTempModel[line][j] = 0;
            break;
        default:
#ifndef M_NO_DEBUG
            qDebug() << "Log in" << __FILE__ << ":" << __FUNCTION__ << " line: " << __LINE__
                     << "AroundValue Over range!" << aroundValue ;
#endif
            break;
        }
    }
}

void MatrixModel::startTransfer()
{
    MatrixSize line = getUpdateLine();

    while (currentStatus() || line != 0)
    {
        transferModelLine(line);
        line = getUpdateLine();
    }

#ifndef M_NO_DEBUG
        if(line != 0)
            qDebug() << "Log in" << __FILE__ << ":" << __FUNCTION__ << " line: " << __LINE__
                     << "Overflow line" << line;
#endif
}

int MatrixModel::getAroundValue(MatrixSize x, MatrixSize y)
{
    MatrixSize around_1X = x != 0 ? x - 1 : modelSize - 1;
    MatrixSize around_1Y = y != 0 ? y - 1 : modelSize - 1;

    MatrixSize around_2X = x;
    MatrixSize around_2Y = y != 0 ? y - 1 : modelSize - 1;

    MatrixSize around_3X = x != modelSize - 1 ? x + 1 : 0;
    MatrixSize around_3Y = y != 0 ? y - 1 : modelSize - 1;

    MatrixSize around_4X = x != 0 ? x - 1 : modelSize - 1;
    MatrixSize around_4Y = y;

    MatrixSize around_6X = x != modelSize - 1 ? x + 1 : 0;
    MatrixSize around_6Y = y;

    MatrixSize around_7X = x != 0 ? x - 1 : modelSize - 1;
    MatrixSize around_7Y = y != modelSize - 1 ? y + 1 : 0;

    MatrixSize around_8X = x;
    MatrixSize around_8Y = y != modelSize - 1 ? y + 1 : 0;

    MatrixSize around_9X = x != modelSize - 1 ? x + 1 : 0;
    MatrixSize around_9Y = y != modelSize - 1 ? y + 1 : 0;

    int aroundValue = currentModel[around_1X][around_1Y]
            + currentModel[around_2X][around_2Y]
            + currentModel[around_3X][around_3Y]
            + currentModel[around_4X][around_4Y]
            + currentModel[around_6X][around_6Y]
            + currentModel[around_7X][around_7Y]
            + currentModel[around_8X][around_8Y]
            + currentModel[around_9X][around_9Y];

    aroundValue += currentModel[x][y] == 0 ? 0 : 10;//大于8表示自身有值

    return aroundValue;
}

void MatrixModel::LFCalculusModelThread()
{
    beginUpdate();
    for(MatrixSize i = 0; i < THREADS; ++i)
    {
        future[i] = QtConcurrent::run(this, &MatrixModel::startCalculus);
    }
    for(MatrixSize i = 0; i < THREADS; ++i)
    {
        future[i].waitForFinished();//等待所有线程结束
    }
#ifndef M_NO_DEBUG
    if(debug != modelSize)
        qDebug() << "Log in" << __FILE__ << ":" << __FUNCTION__ << " line: " << __LINE__
                 << "Thread runs:" << modelSize - debug;
    debug = 0;
#endif

    //把current指向新模型，temp指向旧模型
    int **temp = currentModel;
    currentModel = cTempModel;
    cTempModel = temp;
}

void MatrixModel::changLineAroundValue(MatrixSize line)
{
#ifndef M_NO_DEBUG
    changeMutex.lock();
    ++debug;
    changeMutex.unlock();
#endif

    for(MatrixSize y = 0; y < modelSize; ++y)
    {
        if(currentModel[line][y] != 3 &&
                currentModel[line][y] != 12 &&
                currentModel[line][y] != 13)
        {
            currentModel[line][y] = 0;
            continue;
        }

        MatrixSize around_1X = line != 0 ? line - 1 : modelSize - 1;
        MatrixSize around_1Y = y != 0 ? y - 1 : modelSize - 1;

        MatrixSize around_2X = line;
        MatrixSize around_2Y = y != 0 ? y - 1 : modelSize - 1;

        MatrixSize around_3X = line != modelSize - 1 ? line + 1 : 0;
        MatrixSize around_3Y = y != 0 ? y - 1 : modelSize - 1;

        MatrixSize around_4X = line != 0 ? line - 1 : modelSize - 1;
        MatrixSize around_4Y = y;

        MatrixSize around_6X = line != modelSize - 1 ? line + 1 : 0;
        MatrixSize around_6Y = y;

        MatrixSize around_7X = line != 0 ? line - 1 : modelSize - 1;
        MatrixSize around_7Y = y != modelSize - 1 ? y + 1 : 0;

        MatrixSize around_8X = line;
        MatrixSize around_8Y = y != modelSize - 1 ? y + 1 : 0;

        MatrixSize around_9X = line != modelSize - 1 ? line + 1 : 0;
        MatrixSize around_9Y = y != modelSize - 1 ? y + 1 : 0;

        changeMutex.lock();
        cTempModel[around_1X][around_1Y] += 1;
        cTempModel[around_2X][around_2Y] += 1;
        cTempModel[around_3X][around_3Y] += 1;
        cTempModel[around_4X][around_4Y] += 1;
        cTempModel[line][y] += 10; //值大于等于10则表示对应current有值
        cTempModel[around_6X][around_6Y] += 1;
        cTempModel[around_7X][around_7Y] += 1;
        cTempModel[around_8X][around_8Y] += 1;
        cTempModel[around_9X][around_9Y] += 1;

#ifndef M_NO_DEBUG
        if(cTempModel[line][y] > 18)
            qDebug() << "Log in" << __FILE__ << ":" << __FUNCTION__ << " line: " << __LINE__
                     << "Unclear!" << line << y << cTempModel[line][y];
#endif
        changeMutex.unlock();
        currentModel[line][y] = 0;
    }
}


void MatrixModel::startCalculus()
{
    MatrixSize line = getUpdateLine();

    while (currentStatus() || line != 0)
    {
        changLineAroundValue(line);
        line = getUpdateLine();
    }

#ifndef M_NO_DEBUG
        if(line != 0)
            qDebug() << "Log in" << __FILE__ << ":" << __FUNCTION__ << " line: " << __LINE__
                     << "Overflow line" << line;
#endif
}
