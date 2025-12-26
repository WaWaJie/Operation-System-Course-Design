#pragma once

#include<unordered_map>

enum class ProcessState
{
    Execute,
    Finish,
    StaticBlock,
    ActiveBlock,
    StaticReady,
    ActiveReady
}; 
