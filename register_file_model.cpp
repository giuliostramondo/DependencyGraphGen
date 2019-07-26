#include "register_file_model.hpp"

std::map<int,std::map<int,std::pair<double,double>>> LinearModelArea_test={
    {1, { { 2 , { 3.0, 4.0 }} , {3 , {4.0,5.0}}}},
    {1, { { 2 , { 3.0, 4.0 }}}}
};
std::map<int,std::map<int,std::pair<double,double>>> LinearModelArea={
    {32, {{800, {38.07213005, 38.63783384}}, {1200, {38.05917005, 38.63783384}}, {2000, {39.12266765, 38.67515864}}, {1000, {38.06853828, 38.63761167}}, {1800, {38.91453005, 38.63783384}}, {400, {38.06600183, 38.63805601}}, {1400, {38.05917005, 38.63783384}}, {600, {38.06600183, 38.63805601}}, {1600, {38.05917005, 38.63783384}}}},
    { 64, {{800, {75.1639648, 75.37417649}}, {1200, {75.17596206, 75.37425055}}, {2000, {76.18495354, 75.37987889}},{ 1000, {75.17622126, 75.37373215}},{ 1800, {74.91081897, 75.42779386}},{ 400, {74.32517508, 75.53265878}}, {1400, {75.1637056, 75.37469489}}, {600, {74.62967954, 75.43371843}}, {1600, {75.18122011, 75.37484301}}}}
};
std::map<int,std::map<int,std::pair<double,double>>> LinearModelIdleEnergy={
{32, {{800, {0.00013357, 0.00014357}},{ 1200, {0.00010368, 9.44285714e-05}},{ 2000, {6.64285714e-05, 5.64285714e-05}}, {1000, {8.94642857e-05, 0.00011571}}, {1800, {5.48214286e-05, 6.35714286e-05}},{ 400, {0.00025514, 0.00028614}},{ 1400, {6.45714286e-05, 8.15714286e-05}},{ 600, {0.00015568, 0.00019043}},{ 1600, {8.25714286e-05, 7.05714286e-05}}}},{ 64, {{800, {0.0002465, 0.000282}},{ 1200, {0.00016057, 0.00018957}},{ 2000, {0.00010279, 0.00011229}},{ 1000, {0.00024475, 0.000223}},{ 1800, {0.00014475, 0.000123}},{ 400, {0.00051107, 0.00056357}},{ 1400, {0.00016004, 0.00016029}},{ 600, {0.00038039, 0.00037314}},{ 1600, {0.00013068, 0.00014043}}}}
};
std::map<int,std::map<int,std::pair<double,double>>> LinearModelActiveEnergy={
{32, {{800, {0.00928214, 0.00125714}},{ 1200, {0.01648214, 0.00185714}},{ 2000, {0.02941071, 0.00278571}}, {1000, {0.01561786, 0.00144286}}, {1800, {0.02550714, 0.00255714}},{ 400, {0.00757143, 0.00057143}},{ 1400, {0.01727857, 0.00222857}},{ 600, {0.007675, 0.0009}},{ 1600, {0.02315714, 0.00225714}}}}, {64, {{800, {0.01933929, 0.00221429}},{ 1200, {0.03988214, 0.00255714}}, {2000, {0.05706071, 0.00498571}}, {1000, {0.02731429, 0.00251429}},{ 1800, {0.05006429, 0.00451429}},{ 400, {0.0107, 0.0011}}, {1400, {0.04353571, 0.00328571}},{ 600, {0.016125, 0.0015}}, {1600, {0.04740714, 0.00375714}}}}
};


double getFromModelRegisterFileArea(int bitwidth,int clock, int depth){
    double area_intercept = LinearModelArea[bitwidth][clock].first;
    double area_slope = LinearModelArea[bitwidth][clock].second;
    double area = area_intercept + area_slope * depth;
    return area;
}

double getFromModelRegisterFileIdleEnergy(int bitwidth,int clock, int depth){
    double idle_energy_intercept = LinearModelIdleEnergy[bitwidth][clock].first;
    double idle_energy_slope = LinearModelIdleEnergy[bitwidth][clock].second;
    double idle_energy = idle_energy_intercept + idle_energy_slope * depth;
    return idle_energy;
}

double getFromModelRegisterFileActiveEnergy(int bitwidth,int clock, int depth){
    double active_energy_intercept = LinearModelActiveEnergy[bitwidth][clock].first;
    double active_energy_slope = LinearModelActiveEnergy[bitwidth][clock].second;
    double active_energy = active_energy_intercept + active_energy_slope * depth;
    return active_energy;
}



