/** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** **
* This file is part of winldd
* Copyright (C) 2012 Avinash Dongre ( dongre.avinash@gmail.com )
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** ** **/

#include <Windows.h>
#include <string>
#include <iostream>
#include <exception>
#include <map>

class WinLddException {
public:
  WinLddException(const char* message ) : m_message(message) {
  }
  const char* what() const throw(){
    return m_message.c_str();
  }
private:
  std::string m_message;
};

class WinLdd {
public:
  WinLdd(const char* inFileName) : m_file(inFileName){
  }
  ~WinLdd() {
  }
  void print_dependencies() {
    HMODULE hModule = ::LoadLibraryExA(m_file.c_str(), NULL, DONT_RESOLVE_DLL_REFERENCES);
    if ( ! hModule ) {
      char buffer[1024];
      FormatMessageA( FORMAT_MESSAGE_FROM_SYSTEM, 0, GetLastError(), 0, buffer, 1024, 0 );
      throw WinLddException(buffer);
    }
    IMAGE_IMPORT_DESCRIPTOR* pImportDesc = (IMAGE_IMPORT_DESCRIPTOR*) ((BYTE*)hModule + 
      ((IMAGE_OPTIONAL_HEADER*)((BYTE*)hModule + 
      ((IMAGE_DOS_HEADER*)hModule)->e_lfanew +24))->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

    while ( pImportDesc->FirstThunk ) {
      char* dllName = (char*) ((BYTE*)hModule + pImportDesc->Name);
      std::string dllPath = "";
      HMODULE hModuleDep  = ::LoadLibraryExA(dllName, NULL, DONT_RESOLVE_DLL_REFERENCES);
      if(hModuleDep != 0) {
        char *strDLLPath = new char[_MAX_PATH];
        ::GetModuleFileNameA(hModuleDep, strDLLPath, _MAX_PATH);
        dllPath = std::string(strDLLPath);
        delete strDLLPath;
      }
      FreeLibrary(hModuleDep);
      m_deps[dllName] = dllPath;
      pImportDesc++;
    }
    FreeLibrary(hModule);
    for ( std::map<std::string, std::string>::iterator iter = m_deps.begin(); iter != m_deps.end(); ++iter ) {
      std::cout << "\t" << iter->first << " => " << iter->second << std::endl;
    }
  }
private:
  std::string m_file;
  std::map<std::string, std::string> m_deps;
protected:
};

int main(int argc, char** argv) {
  if(argc <= 1) {
    std::cout << "usage:" << std::endl <<  "winldd <shared dll file name>" << std::endl;
    exit (1);
  }
  try {
    WinLdd wldd(argv[1]);
    wldd.print_dependencies();
  } catch (WinLddException &expt) {
    std::cout << expt.what() << std::endl;
    exit(1);
  }
  return 0;
}



