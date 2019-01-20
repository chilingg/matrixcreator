#include "matrixmodel.h"

MatrixModel::MatrixModel(unsigned size, ModelPattern pattern):
    traceOnOff(false),
    THREADS(std::thread::hardware_concurrency()),
    future(THREADS),	//获取cpu核数
    currentModel(size, vector<int>(size, 0)),
    modelPattern(EmptyPattern),
    modelSize(size),
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
                unTracedUnit(x+i, y+j, tracedUnit);
                currentModel[x+i][y+j] = 0;
            }
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
    case LifeGameTSF:
        tTempModel.clear();
        break;
    case LifeGameCCL:
        cTempModel.clear();
        break;
    case LifeGameTRC:
        trTempModel.clear();
        tracedUnit[0].clear();
        tracedUnit[1].clear();
        tracedUnit[2].clear();
        traceOnOff = false;
        THREADS = std::thread::hardware_concurrency();
        break;
    default:
        break;
    }

    //开始新的模式
    switch (after) {
    case LifeGameTSF:
        tTempModel.assign(modelSize, vector<int>(modelSize, 0));
        updateModel = &MatrixModel::LFTransferModelThread;
        break;
    case LifeGameCCL:
        cTempModel.assign(modelSize, vector<int>(modelSize, 0));
        updateModel = &MatrixModel::LFCalculusModelThread;
        break;
    case LifeGameTRC:
        trTempModel.assign(modelSize, vector<int>(modelSize, 0));
        updateModel = &MatrixModel::LFTraceModelThread;
        traceOnOff = true;
        THREADS = 1;
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
    swap(currentModel, tTempModel);
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

void MatrixModel::LFTraceModelThread()
{
    for(MatrixSize i = 0; i < THREADS; ++i)
    {
        future[i] = QtConcurrent::run(this, &MatrixModel::startTrace);
    }
    for(MatrixSize i = 0; i < THREADS; ++i)
    {
        future[i].waitForFinished();//等待所有线程结束
    }

    //把current指向新模型，temp指向旧模型
    swap(currentModel, trTempModel);
    swap(tracedUnit, tempTrace);
}

void MatrixModel::changTraceAroundValue(TraceLine line)
{
    MatrixSize column = line.first;

    for(const auto &row : line.second)
    {
        if(currentModel[column][row] != 3 &&
                currentModel[column][row] != 12 &&
                currentModel[column][row] != 13)
        {
            currentModel[column][row] = 0;
            continue;
        }

        MatrixSize leftX = column != 0 ? column - 1 : modelSize - 1;
        MatrixSize rightX = column != modelSize - 1 ? column + 1 : 0;
        MatrixSize topY = row != 0 ? row - 1 : modelSize - 1;
        MatrixSize bottomY = row != modelSize - 1 ? row + 1 : 0;

        trTempModel[leftX][topY] += 1;
        trTempModel[column][topY] += 1;
        trTempModel[rightX][topY] += 1;

        trTempModel[leftX][row] += 1;
        trTempModel[column][row] += 10; //值大于等于10则表示对应current有值
        trTempModel[rightX][row] += 1;

        trTempModel[leftX][bottomY] += 1;
        trTempModel[column][bottomY] += 1;
        trTempModel[rightX][bottomY] += 1;

        //追踪有值的单元
        tracedUnitTMB(leftX, row, tempTrace);
        tracedUnitTMB(column, row, tempTrace);
        tracedUnitTMB(rightX, row, tempTrace);

#ifndef M_NO_DEBUG
        if(trTempModel[column][row] > 18)
            qDebug() << "Log in" << __FILE__ << ":" << __FUNCTION__ << " line: " << __LINE__
                     << "Unclear!" << column << row << trTempModel[column][row];
#endif
        currentModel[column][row] = 0;
    }
}

void MatrixModel::startTrace()
{
    static unsigned threadSum = 0;
    static QMutex mutex;
    TraceLine end = make_pair(modelSize, set<MatrixSize>());
    size_t index = 0;

    do{
        TraceLine line = popTracedLine(index);
        while (line != end)
        {
            changTraceAroundValue(line);
            line = popTracedLine(index);
        }

        //同步线程
        mutex.lock();
        ++threadSum;
        if(threadSum == THREADS)
        {
            threadSum = 0;
            synchroThread.wakeAll();//唤醒全部线程

            //例外的列最多维两列，用一个线程处理
            if(index == 2)
            {
                TraceLine line = popTracedLine(3);
                while (line != end)
                {
                    changTraceAroundValue(line);
                    line = popTracedLine(3);
                }
            }
        }
        else {
            synchroThread.wait(&mutex);//线程等待
        }
        mutex.unlock();

    } while(++index < 3);
}

TraceLine MatrixModel::popTracedLine(size_t index)
{
    //QMutexLocker创建时锁定资源，析构时解锁后其它线程才能进入
    QMutexLocker locker(&lineMutex);

    if(tracedUnit[index].empty())
    {
        return make_pair(modelSize, set<MatrixSize>());
    }

    TraceLine line = *tracedUnit[index].begin();
    tracedUnit[index].erase(line.first);

    return line;
}

void MatrixModel::traceUnit(MatrixSize column, MatrixSize row, map<MatrixSize, set<MatrixSize>> *trace)
{
    //Size非3倍数时，会对首列干扰的尾列
    if(column >= (modelSize/3)*3)
    {
        auto it = trace[3].find(column);
        if(it != trace[3].end())
        {
            it->second.insert(row);
        }
        else
        {
            trace[3].insert(make_pair(column, set<MatrixSize>({row})));
        }
        return;
    }

    size_t i = column % 3;
    auto it = trace[i].find(column);
    if(it != trace[i].end())
    {
        it->second.insert(row);
    }
    else
    {
        trace[i].insert(make_pair(column, set<MatrixSize>({row})));
    }
}

void MatrixModel::unTracedUnit(MatrixSize column, MatrixSize row, map<MatrixSize, set<MatrixSize>> *trace)
{
    size_t i = column % 3;
    auto it = trace[i].find(column);
    if(it != trace[i].end())
    {
        it->second.erase(row);
    }
}

void MatrixModel::tracedUnitTMB(MatrixSize column, MatrixSize rowM, map<MatrixSize, set<MatrixSize> > *trace)
{
    MatrixSize rowT = rowM != 0 ? rowM - 1 : modelSize - 1;
    MatrixSize rowB = rowM != modelSize - 1 ? rowM + 1 : 0;

    //Size非3倍数时，会对首列干扰的尾列
    if(column >= (modelSize/3)*3)
    {
        auto it = trace[3].insert(make_pair(column, set<MatrixSize>({rowM})));
        if(it.second)
        {
            it.first->second.emplace(rowT);
            it.first->second.emplace(rowB);
        }
        else
        {
            it.first->second.emplace(rowM);
            it.first->second.emplace(rowT);
            it.first->second.emplace(rowB);
        }
        return;
    }

    size_t i = column % 3;
    auto it = trace[i].insert(make_pair(column, set<MatrixSize>({rowM})));
    if(it.second)
    {
        it.first->second.emplace(rowT);
        it.first->second.emplace(rowB);
    }
    else
    {
        it.first->second.emplace(rowM);
        it.first->second.emplace(rowT);
        it.first->second.emplace(rowB);
    }
    return;
}
