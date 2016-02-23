#!/usr/bin/env python


import platform
import re
import sys
import os
import fnmatch
import optparse

def DoMain(argv):
  parser = optparse.OptionParser("usage: %prog absPath")
  absPath = parser.parse_args(argv)
  if not absPath:
    parser.error("Please specify the absolute path")
   
  absPath = absPath[1]
  if not absPath:
    parser.error("Please specify the absolute path")
  
  absPath = absPath[0]
  if not absPath:
    parser.error("Please specify the absolute path")
  
  if os.path.isabs(absPath):
    return os.path.relpath(absPath)
    
if __name__ == '__main__':
  results = DoMain(sys.argv[1:])
  if results:
    print results