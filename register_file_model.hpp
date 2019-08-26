#ifndef REGISTER_FILE_MODEL_HPP
#define REGISTER_FILE_MODEL_HPP

#include<map>
#include<utility>


//These call return 0 if the information regarding the specific register file are not available
double getFromModelRegisterFileArea(int bitwidth,int clock, int depth,int technology);
double getFromModelRegisterFileIdleEnergy(int bitwidth,int clock, int depth,int technology);
double getFromModelRegisterFileActiveEnergy(int bitwidth,int clock, int dept,int technology);

#endif
