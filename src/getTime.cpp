#include <TimeLib.h>
///Шляпа получится если запустить эту ночью перед 12 00


int getTimeInMin(){
    return hour() * 60 + minute();
}


int getTimeInSec(){

    return minute() * 60 + second();
}