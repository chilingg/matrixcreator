#include <QDebug>
#include <QtConcurrent>
#include "matrixmodel.h"

MatrixModel::MatrixModel():
    cModel_(new int[WORLDSIZE * WORLDSIZE]),
    tModel_(new int[WORLDSIZE * WORLDSIZE]),
    updateIndex_(0),
    update_(false),
    done_(false)
{
    std::fill_n(cModel_, WORLDSIZE * WORLDSIZE, 0);
    std::fill_n(tModel_, WORLDSIZE * WORLDSIZE, 0);

    unsigned tNum = std::thread::hardware_concurrency() > 1 ? std::thread::hardware_concurrency() - 1 : 0;
    threads_.reserve(tNum);

    for(unsigned i = 0; i < tNum; ++i)
    {
        threads_.emplace_back([this]{
            while(!done_)
            {
                if(update_)
                    update();
                else
                    std::this_thread::yield();
            }
        });
    }
}

MatrixModel::~MatrixModel()
{
    done_ = true;
    for(auto & thread : threads_)
        thread.join();

    delete [] cModel_;
    delete [] tModel_;
}

int MatrixModel::value(int x, int y)
{
    return cModel_[y * WORLDSIZE + x];
}

void MatrixModel::updateModel()
{
    for(size_t i = 0; i < WORLDSIZE; ++i)
    {
        size_t y = i * WORLDSIZE;
        for(size_t x = 0; x < WORLDSIZE; ++x)
        {
            size_t t = (i == 0 ? WORLDSIZE - 1 : i - 1) * WORLDSIZE;
            size_t b = (i == WORLDSIZE - 1 ? 0 : i + 1) * WORLDSIZE;

            size_t l = x == 0 ? WORLDSIZE - 1 : x - 1;
            size_t r = x == WORLDSIZE - 1 ? 0 : x + 1;

            tModel_[y + x] = lifeGameRule(cModel_[t + l], cModel_[t + x], cModel_[t + r],
                    cModel_[y + l], cModel_[y + x], cModel_[y + r],
                    cModel_[b + l], cModel_[b + x], cModel_[b + r]);
        }
    }

    std::swap(cModel_, tModel_);
}

void MatrixModel::updateInThread()
{
    update_ = true;
    update();

    std::swap(cModel_, tModel_);
    updateIndex_ = 0;
}

void MatrixModel::setValue(size_t x, size_t y, int value)
{
    cModel_[x + y * WORLDSIZE] = value;
}

void MatrixModel::setRangeValue(size_t x, size_t y, size_t widht, size_t height, int value)
{
# ifndef NDEBUG
    if(x + widht > WORLDSIZE || y + height > WORLDSIZE)
        throw "Invalid range!";
#endif
    for(size_t i = y; i < height + y; ++i)
    {
        for(size_t j = x; j < widht + x; ++j)
        {
            cModel_[i * WORLDSIZE + j] = value;
        }
    }
}

void MatrixModel::fill(int value)
{
    std::fill_n(cModel_, WORLDSIZE * WORLDSIZE, value);
}

void MatrixModel::clear()
{
    std::fill_n(cModel_, WORLDSIZE * WORLDSIZE, 0);
}

int MatrixModel::lifeGameRule(int tl, int tm, int tr, int ml, int mm, int mr, int bl, int bm, int br)
{
    int around = tl + tm + tr + ml + mr + bl + bm + br;

    switch (around)
    {
    case 0:
    case 1:
        return 0;
    case 2:
        return mm;
    case 3:
        return 1;
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
        return 0;
    default:
        throw "Invalid value!";
    }
}

void MatrixModel::update()
{
    for(size_t i = updateIndex_++; i < WORLDSIZE; i = updateIndex_++)
    {
        size_t y = i * WORLDSIZE;
        for(size_t x = 0; x < WORLDSIZE; ++x)
        {
            size_t t = (i == 0 ? WORLDSIZE - 1 : i - 1) * WORLDSIZE;
            size_t b = (i == WORLDSIZE - 1 ? 0 : i + 1) * WORLDSIZE;

            size_t l = x == 0 ? WORLDSIZE - 1 : x - 1;
            size_t r = x == WORLDSIZE - 1 ? 0 : x + 1;

            tModel_[y + x] = lifeGameRule(cModel_[t + l], cModel_[t + x], cModel_[t + r],
                    cModel_[y + l], cModel_[y + x], cModel_[y + r],
                    cModel_[b + l], cModel_[b + x], cModel_[b + r]);
        }
    }

    bool b = true;
    update_.compare_exchange_weak(b, false);
}
