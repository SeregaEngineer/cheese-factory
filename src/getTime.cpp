#include <TimeLib.h>
///Шляпа получится если запустить эту ночью перед 12 00


int getTimeInMin(){
    //return hour() * 60 + minute();
    return now();
}


int getTimeInSec(){
    //return minute() * 60 + second();
    return now();
}