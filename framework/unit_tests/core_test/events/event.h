#pragma once

#include <string>

struct T1                                                                          
{};                                                                                
                                                                                   
struct T2                                                                          
{};                                                                                
                                                                                   
struct Value                                                                       
{                                                                                  
  int value{-1};                                                                   
};                                                                                 
                                                                                   
template <typename Event>                                                          
struct Tag                                                                         
{                                                                                  
  Event data;                                                                      
  std::string tag;                                                                 
};                                                                                 
                                                                                   
struct WaitPerk                                                                    
{                                                                                  
                                                                                   
};
