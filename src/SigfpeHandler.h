// Copyright (C) 2014-2017 Hideaki Narita


#ifndef IKURA_SIGFPEHANDLER_H
#define IKURA_SIGFPEHANDLER_H


#include <setjmp.h>
#include <signal.h>


namespace hnrt
{
    //
    // SIGFPE signal handler
    //
    // How to use:
    //
    // SigfpeHandler handler;
    // ...
    // handler.resetCode(); // SIGFPE unblocked
    // if (sigsetjmp(SigfpeHandler::env, 1) == 0)
    // {
    //   do some arithmetic operation;
    // }
    // switch (sigfpeHandler.getCode()) // SIGFPE blocked
    // {
    // case FPE_INTDIV: // integer divide by zero
    // case FPE_FLTDIV: // floating-point divide by zero
    // case FPE_INTOVF: // integer overflow
    // case FPE_FLTOVF: // floating-point overflow
    // case FPE_FLTUND: // floating-point underflow
    // case FPE_FLTRES: // floating-point inexact result
    // case FPE_FLTINV: // floating-point invalid operation
    // case FPE_FLTSUB: // subscript out of range
    //     do error processing;
    //     break;
    // default: // no error
    //     break;
    // }
    //
    class SigfpeHandler
    {
    public:

        static sigjmp_buf env;

        SigfpeHandler();
        ~SigfpeHandler();
        void resetCode();
        int getCode();

    private:

        static void handler(int no, siginfo_t *si, void *uc);

        SigfpeHandler(const SigfpeHandler&) {}

        static volatile int code;

        sigset_t ssFpe;
        sigset_t ssOld;
        struct sigaction saFpe;
        struct sigaction saOld;
    };
}


#endif //!IKURA_SIGFPEHANDLER_H
