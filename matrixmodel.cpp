#include "matrixmodel.h"

MatrixModel::MatrixModel(int )
{
    //初始化模型，暂为偶数0，奇数1 <==Test
    int k = 0;
    for(int i = 0; i < WORLDSIZE; ++i)
    {
        i % 2 == 0 ? k = 0 : k = 1; //每轮换一行则变更单双规则
        for(int j = 0; j < WORLDSIZE; ++j)
        {
            if(k++ % 2 == 0)
                model[i][j] = 0;
            else
                model[i][j] = 1;
        }
    }
}

int MatrixModel::getModelValue(int x, int y)
{
    return (model[x][y]);

}

void MatrixModel::updateModel()
{
    //更新模型，奇变偶 偶变奇 <==Test
    for(int i = 0; i < WORLDSIZE; ++i)
    {
        for(int j = 0; j < WORLDSIZE; ++j)
        {
            if (model[i][j] % 2 == 0)
                model[i][j] = 1;
            else
                model[i][j] = 0;
        }
    }
}
