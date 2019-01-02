#ifndef MPOINT_H
#define MPOINT_H

#include <QPoint>
#include <QRect>

struct MPoint
{
    bool valid;
    int modelColumn;
    int modelRow;
    QPoint clickted;
    QRect viewRect;
};

#endif // MPOINT_H
