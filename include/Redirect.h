#ifndef TREDIRECT_H
#define TREDIRECT_H

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <fcntl.h>
#include <unistd.h>

/////////////////////////////////////////////////////////////////
///
/// \class Redirect
///
/// A simple class to redirect stdout and stderr to file(s). To
/// use it, create it, providing the path of the files stdout and
/// stderr should be redirected to, and an (optional) flag whether
/// to append to those files (default) or not.
/// If only one file is provided, both stdout and stderr are
/// redirected to it.
/// If the filename for either stdout or stderr is a nullptr, the
/// redirection for this output won't happen.
/// The redirection ends when the Redirect object is destroyed.
///
/////////////////////////////////////////////////////////////////

class Redirect {
public:
   Redirect(const char* newOut, const char* newErr, bool append = true)
      : fOutFile(newOut), fErrFile(newErr)
   {
      Run(append);
   }
   explicit Redirect(const char* newOut, bool append = true)
      : fOutFile(newOut), fErrFile(newOut)
   {
      Run(append);
   }

   const char* OutFile() { return fOutFile; }
   const char* ErrFile() { return fErrFile; }

   ~Redirect()
   {
      fflush(stdout);
      dup2(fStdOutFileDescriptor, fileno(stdout));
      fflush(stderr);
      dup2(fStdErrFileDescriptor, fileno(stderr));
   }

   Redirect(const Redirect&) = delete;
   Redirect(Redirect&&)      = delete;

   Redirect& operator=(const Redirect&) = delete;
   Redirect& operator=(Redirect&&)      = delete;

private:
   void Run(bool append)
   {
      if(fOutFile != nullptr) {
         fStdOutFileDescriptor = dup(fileno(stdout));
         fflush(stdout);
         int newStdOut = open(fOutFile, (append ? O_WRONLY | O_CREAT | O_APPEND : O_WRONLY | O_CREAT), S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
         dup2(newStdOut, fileno(stdout));
         close(newStdOut);
      }
      if(fErrFile != nullptr) {
         fStdErrFileDescriptor = dup(fileno(stderr));
         fflush(stderr);
         int newStdErr = open(fErrFile, (append ? O_WRONLY | O_CREAT | O_APPEND : O_WRONLY | O_CREAT), S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
         dup2(newStdErr, fileno(stderr));
         close(newStdErr);
      }
   }

   int         fStdOutFileDescriptor{0};
   int         fStdErrFileDescriptor{0};
   const char* fOutFile{nullptr};
   const char* fErrFile{nullptr};
};

#endif
