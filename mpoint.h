#ifndef MPOINT_H
#define MPOINT_H

#include <QPoint>

struct MPoint
{
    bool valid;
    unsigned viewX;
    unsigned viewY;
    unsigned modelColumn;
    unsigned modelRow;
    QPoint clickted;
};

#endif // MPOINT_H
