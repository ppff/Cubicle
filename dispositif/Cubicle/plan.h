#ifndef PLAN_H
#define PLAN_H

#include <QObject>
#include<QVector>
#include<led.h>
class Plan
{
public:
    Plan();

private:


   QVector<Led> led_list[9];

};

#endif // PLAN_H
