#include "matrixmodel.h"

MatrixModel::MatrixModel(unsigned size, ModelPattern pattern):
    THREADS(std::thread::hardware_concurrency()),
    future(THREADS),	//获取cpu核数
    currentModel(size, vector<int>(size, 0)),
    modelPattern(EmptyPattern),
    modelSize(size),
    traceOnOff(false),
    updateStatus(false)
{
    modelPattern = switchModel(pattern);
}

MatrixModel::~MatrixModel()
{
    switchModel(EmptyPattern);
    currentModel.clear();
}

void MatrixModel::clearUnit(MatrixSize x, MatrixSize y, MatrixSize widht, MatrixSize height)
{
#ifndef M_NO_DEBUG
    if(x+widht > modelSize || y+height > modelSize)
        qDebug() << "Log in" << __FILE__ << ":" << __FUNCTION__ << " line: " << __LINE__
                 << "error: Index out of range!";
#endif

    for(MatrixSize i = 0; i < widht; ++i)
    {
        for(MatrixSize j = 0; j < height; ++j)
        {
            if(currentModel[x+i][y+j] != 0)
            {
                unTracedUnit(x+i, y+j, traceUnit);
                currentModel[x+i][y+j] = 0;
            }
            if(tTempModel[x+i][y+j] != 0)
                tTempModel[x+i][y+j] = 0;
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
        tTempModel.clear();
        traceUnit.clear();
        traceOnOff = false;
        break;
    case LifeGame:
        cTempModel.clear();
        break;
    default:
        break;
    }

    //开始新的模式
    switch (after) {
    case LifeGameT:
        tTempModel.assign(modelSize, vector<int>(modelSize, 0));
        traceOnOff = true;
        updateModel = &MatrixModel::LFTransferModelThread;
        break;
    case LifeGame:
        cTempModel.assign(modelSize, vector<int>(modelSize, 0));
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
    static size_t line = 0;

    if(!updateStatus)
        return 0;

    if(line >= modelSize)
    {
        line = 0;
        updateStatus = false;
        return 0;
    }

    return line++;
}

MatrixSize MatrixModel::getUpdateLine(MatrixSize interval, MatrixSize pos)
{
    //QMutexLocker创建时锁定资源，析构时解锁后其它线程才能进入
    QMutexLocker locker(&lineMutex);
    static MatrixSize line = 0;
    static MatrixSize sum = 0;

    if(!updateStatus)
        return pos;

    line = sum * interval + pos;
    if(line >= modelSize)
    {
        line = 0;
        sum = 0;
        updateStatus = false;
        return pos;
    }

    ++sum;
    return line;
}

void MatrixModel::LFTransferModelThread()
{
    for(MatrixSize i = 0; i < THREADS; ++i)
    {
        future[i] = QtConcurrent::run(this, &MatrixModel::startTransfer);
    }
    for(MatrixSize i = 0; i < THREADS; ++i)
    {
        future[i].waitForFinished();//等待所有线程结束
    }

    //把current指向新模型，temp指向旧模型
    swap(currentModel, tTempModel);
    swap(traceUnit, traceUnit2);
}

void MatrixModel::transferModelLine(MatrixSize column, MatrixSize row)
{
    int aroundValue = getAroundValue(column, row);

    //计算出的模型存在tempModel中，避免影响正在进行的getAroundValue计算
    tTempModel[column][row] = currentModel[column][row];
    switch (aroundValue)
    {
    case 0:
    case 1:
    case 2:
        break;
    case 3:
        tTempModel[column][row] = 1;
        changeMutex.lock();
        tracedUnit(column, row, traceUnit2);
        changeMutex.unlock();
        break;
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
    case 10:
    case 11:
        tTempModel[column][row] = 0;
        break;
    case 12:
    case 13:
        changeMutex.lock();
        tracedUnit(column, row, traceUnit2);
        changeMutex.unlock();
        break;
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
        tTempModel[column][row] = 0;
        break;
    default:
#ifndef M_NO_DEBUG
        qDebug() << "Log in" << __FILE__ << ":" << __FUNCTION__ << " line: " << __LINE__
                 << "AroundValue Over range!" << aroundValue ;
#endif
        break;
    }
}

void MatrixModel::startTransfer()
{
    UnitPoint end = std::make_pair(modelSize, modelSize);

    set<UnitPoint>::iterator uit = tracersalTracedUnit();
    while (uit != traceUnit.end())
    {
        transferModelLine(uit->first, uit->second);
        uit = tracersalTracedUnit();
    }

    //同步线程
    changeMutex.lock();
    static unsigned threadSum = 0;
    ++threadSum;
    if(threadSum == THREADS)
    {
        synchroThread.wakeAll();//唤醒全部线程
        threadSum = 0;
    }
    else {
        synchroThread.wait(&changeMutex);//线程等待
    }
    changeMutex.unlock();

    UnitPoint point = popTracedUnit();
    while (point != end)
    {
        currentModel[point.first][point.second] = 0;
        point = popTracedUnit();
    }

#ifndef M_NO_DEBUG
    if(point != end)
        qDebug() << "Log in" << __FILE__ << ":" << __FUNCTION__ << " line: " << __LINE__
                 << "Overflow line" << point;
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
    swap(currentModel, cTempModel);
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

        //changeMutex.lock();
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
        //changeMutex.unlock();
        currentModel[line][y] = 0;
    }
}


void MatrixModel::startCalculus()
{
    static unsigned threadSum = 0;
    static const unsigned intarval = 3;//在区间内修改模型是线程安全的
    MatrixSize offset = 0;//逐步增加区间内的偏移值

    do{
        MatrixSize line = getUpdateLine(intarval, offset);

        while (currentStatus() || line != offset)
        {
            changLineAroundValue(line);
            line = getUpdateLine(intarval, offset);
        }

        //同步线程
        changeMutex.lock();
        ++threadSum;
        if(threadSum == THREADS)
        {
            synchroThread.wakeAll();//唤醒全部线程
            threadSum = 0;
            beginUpdate();
        }
        else {
            synchroThread.wait(&changeMutex);//线程等待
        }
        changeMutex.unlock();

    }while (++offset < intarval);
}

UnitPoint MatrixModel::popTracedUnit()
{
    //QMutexLocker创建时锁定资源，析构时解锁后其它线程才能进入
    QMutexLocker locker(&lineMutex);

    if(traceUnit.empty())
    {
        return {modelSize, modelSize};
    }

    UnitPoint point = *traceUnit.begin();
    traceUnit.erase(point);

    return point;
}

void MatrixModel::tracedUnit(MatrixSize column, MatrixSize row, set<UnitPoint> &trace)
{
    if(traceOnOff)
    {
        MatrixSize around_1X = column != 0 ? column - 1 : modelSize - 1;
        MatrixSize around_1Y = row != 0 ? row - 1 : modelSize - 1;

        MatrixSize around_2X = column;
        MatrixSize around_2Y = row != 0 ? row - 1 : modelSize - 1;

        MatrixSize around_3X = column != modelSize - 1 ? column + 1 : 0;
        MatrixSize around_3Y = row != 0 ? row - 1 : modelSize - 1;

        MatrixSize around_4X = column != 0 ? column - 1 : modelSize - 1;
        MatrixSize around_4Y = row;

        MatrixSize around_6X = column != modelSize - 1 ? column + 1 : 0;
        MatrixSize around_6Y = row;

        MatrixSize around_7X = column != 0 ? column - 1 : modelSize - 1;
        MatrixSize around_7Y = row != modelSize - 1 ? row + 1 : 0;

        MatrixSize around_8X = column;
        MatrixSize around_8Y = row != modelSize - 1 ? row + 1 : 0;

        MatrixSize around_9X = column != modelSize - 1 ? column + 1 : 0;
        MatrixSize around_9Y = row != modelSize - 1 ? row + 1 : 0;

        trace.insert({around_1X, around_1Y});
        trace.insert({around_2X, around_2Y});
        trace.insert({around_3X, around_3Y});
        trace.insert({around_4X, around_4Y});
        trace.insert({column, row});
        trace.insert({around_6X, around_6Y});
        trace.insert({around_7X, around_7Y});
        trace.insert({around_8X, around_8Y});
        trace.insert({around_9X, around_9Y});
    }
}

void MatrixModel::unTracedUnit(MatrixSize column, MatrixSize row, set<UnitPoint> &trace)
{
    if(traceOnOff)
    {
        MatrixSize around_1X = column != 0 ? column - 1 : modelSize - 1;
        MatrixSize around_1Y = row != 0 ? row - 1 : modelSize - 1;

        MatrixSize around_2X = column;
        MatrixSize around_2Y = row != 0 ? row - 1 : modelSize - 1;

        MatrixSize around_3X = column != modelSize - 1 ? column + 1 : 0;
        MatrixSize around_3Y = row != 0 ? row - 1 : modelSize - 1;

        MatrixSize around_4X = column != 0 ? column - 1 : modelSize - 1;
        MatrixSize around_4Y = row;

        MatrixSize around_6X = column != modelSize - 1 ? column + 1 : 0;
        MatrixSize around_6Y = row;

        MatrixSize around_7X = column != 0 ? column - 1 : modelSize - 1;
        MatrixSize around_7Y = row != modelSize - 1 ? row + 1 : 0;

        MatrixSize around_8X = column;
        MatrixSize around_8Y = row != modelSize - 1 ? row + 1 : 0;

        MatrixSize around_9X = column != modelSize - 1 ? column + 1 : 0;
        MatrixSize around_9Y = row != modelSize - 1 ? row + 1 : 0;

        trace.erase({around_1X, around_1Y});
        trace.erase({around_2X, around_2Y});
        trace.erase({around_3X, around_3Y});
        trace.erase({around_4X, around_4Y});
        trace.erase({column, row});
        trace.erase({around_6X, around_6Y});
        trace.erase({around_7X, around_7Y});
        trace.erase({around_8X, around_8Y});
        trace.erase({around_9X, around_9Y});
    }
}

set<UnitPoint>::iterator MatrixModel::tracersalTracedUnit()
{
    //QMutexLocker创建时锁定资源，析构时解锁后其它线程才能进入
    QMutexLocker locker(&lineMutex);
    static bool frist = true;
    static set<UnitPoint>::iterator it;

    if(frist)
    {
        it = traceUnit.begin();
        frist = false;
    }

    if(it == traceUnit.end())
    {
        frist = true;
        updateStatus = false;
        return it;
    }

    return it++;
}
