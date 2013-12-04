#include <cstdlib>
//#include <stdlib.h>
#include <getopt.h>
#include "ackerman.h"
#include "my_allocator.h"
#include <iostream>
#include <stdio.h>
#include <signal.h>

void doExitMethods(){
  release_allocator();
  std::cout << "Allocator's memory has been released. Memory test has been exited. Thank you for using it.\n";
  std::cout.flush();
  _exit(1);
}
void onExit(){
  doExitMethods();
}
void onSignalExit(int n){
  doExitMethods();
}

int main(int argc, char ** argv) {

  std::atexit(onExit);
  signal(SIGABRT, onSignalExit);
  signal(SIGINT, onSignalExit);
  signal(SIGTERM, onSignalExit);

  int b = -1; int s = -1;

  int opt = getopt(argc, argv, "b:s:");

  while(opt != -1){
    switch(opt){
      case 'b':
        b = atoi(optarg);
        break;
      case 's':
        s = atoi(optarg);
        break;
    }
    opt = getopt(argc, argv, "b:s:");
  }

  if(b != -1 && s != -1){
    init_allocator(b,s);
  }
  else{
    init_allocator(128, 512*1024);
  }

  ackerman_main();
}
