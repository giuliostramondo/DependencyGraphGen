#ifndef REGISTER_FILE_MODEL_HPP
#define REGISTER_FILE_MODEL_HPP

#include<map>
#include<utility>

double getFromModelRegisterFileArea(int bitwidth,int clock, int depth);
double getFromModelRegisterFileIdleEnergy(int bitwidth,int clock, int depth);
double getFromModelRegisterFileActiveEnergy(int bitwidth,int clock, int depth);

#endif
