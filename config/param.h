#ifndef _PARMA_H_
#define _PARMA_H_
#include "/home/nuc/Documents/zz/gesture_detect/src/utility/include/fileToolkit.hpp"
#include <iostream>
extern filetoolkit::FILETOOLKIT file;

#define KP std::stof(file.paramDict["KP"])
#define KI std::stof(file.paramDict["KI"])
#define KD std::stof(file.paramDict["KD"])
#endif // _PARMA_H_