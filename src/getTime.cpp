#include <TimeLib.h>
///Хуйня получится если запустить эту залупу ночью перед 12 00


int getTimeInMin(){
    return hour() * 60 + minute();
}


int getTimeInSec(){

    return minute() * 60 + second();
}