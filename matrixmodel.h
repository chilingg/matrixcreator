#ifndef MATRIXMODEL_H
#define MATRIXMODEL_H

#include "constants.h"
#include <array>
#include <vector>
#include <atomic>
#include <functional>
#include <thread>

class MatrixModel
{
public:
    MatrixModel();
    ~MatrixModel() = default;

    int value(int x, int y); //依据坐标返回单个数据

    void updateModel(); //更新数据

    void setValue(size_t x, size_t y, int value); //修改模型的值
    void setRangeValue(size_t x, size_t y, size_t widht, size_t height, int value);
    void fill(int value);
    void clear();

private:
    static int lifeGameRule(int tl, int tm, int tr, int ml, int mm, int mr, int bl, int bm, int br);

    void update();

    std::atomic_size_t updateIndex_;
    std::function<int(int, int, int, int, int, int, int, int, int)> updateFunc_;
    std::vector<std::thread> threads_;
    std::array<int, WORLDSIZE * WORLDSIZE> cModel_;
    std::array<int, WORLDSIZE * WORLDSIZE> tModel_;
};

#endif // MATRIXMODEL_H
